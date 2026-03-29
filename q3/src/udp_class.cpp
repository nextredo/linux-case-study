#include "udp_class.hpp"

UdpBroker::~UdpBroker()
{
    // Tell workers they should stop working
    _workerExecFlag = false;

    // Wake all workers in condition-var based waiting operations
    _workerCondVar.notify_all();

    for (auto& thread : _workers)
    {
        if (thread.joinable())
            thread.join();
    }
}

std::string UdpBroker::decodeIp(struct sockaddr_storage* sa)
{
    std::string str;
    auto af = sa->ss_family;
    void* ip = nullptr;

    switch (af)
    {
        case AF_INET:
            str.resize(INET_ADDRSTRLEN);
            ip = &((struct sockaddr_in*)sa)->sin_addr;
            break;

        case AF_INET6:
            str.resize(INET6_ADDRSTRLEN);
            ip = &((struct sockaddr_in6*)sa)->sin6_addr;
            break;

        default:
            break;
    };

    inet_ntop(af, ip, str.data(), str.size());

    // Truncate string object to length of null-terminated contents
    str.resize(std::strlen(str.c_str()));
    return str;
}

// TODO allow socket re-binding without errors
// int yes=1;
// char yes='1'; // Solaris people use this
// lose the pesky "Address already in use" error message
// setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

ssize_t UdpBroker::recv(const char* port, void* data, const size_t len,
        struct sockaddr_storage* senderAddr, socklen_t* senderAddrLen,
        const ip_ver_e ipVer, std::chrono::seconds timeout)
{
    // Init struct to default values (brace --> value initialisation)
    struct addrinfo hints {};
    struct addrinfo* result = nullptr;

    // TODO combine with send() setup
        // move getaddrinfo and socket creation into a common area

    // Alternatively, can spawn an IPv6 socket to listen for all
    // incoming connections and receive IPv4 traffic in mapped address format
    hints.ai_family   = ipVer;   // Use any address family (IPv4 or IPv6 here)
    hints.ai_socktype = SOCK_DGRAM;  // Only return datagram type sockets
    hints.ai_flags    = AI_PASSIVE;  // Autofill my IP socket (used for incoming connections)
    hints.ai_protocol = IPPROTO_UDP; // Only return sockets for the UDP protocol

    int socket_fd       = 0;
    int gai_ret         = 0;
    int bind_ret        = 0;
    int sockopt_ret     = 0;
    int listen_ret      = 0;
    ssize_t bytes_recvd = -1;

    timeval sock_timeout = { .tv_sec = timeout.count() };

    // TODO
    // int yes=1;
    // setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    gai_ret = getaddrinfo(nullptr, port, &hints, &result);
    if (gai_ret < 0)
    {
        fprintf(stderr, "Reception: get addr info error: %s\n", gai_strerror(gai_ret));
        goto cleanup;
    }

    // WARN: Ignoring possibility of multiple address information
    // returns from getaddrinfo() in addrinfo struct

    errno = 0;
    socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (socket_fd == -1)
    {
        perror("Reception socket creation");
        goto cleanup;
    }

    errno = 0;
    bind_ret = bind(socket_fd, result->ai_addr, result->ai_addrlen);
    if (bind_ret == -1)
    {
        perror("Reception socket bind");
        goto cleanup;
    }

    errno = 0;
    sockopt_ret = setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &sock_timeout, sizeof(sock_timeout));
    if (sockopt_ret == -1)
    {
        perror("Reception socket options");
        goto cleanup;
    }

    errno = 0;
    bytes_recvd = recvfrom(socket_fd, data, len, 0,
            (struct sockaddr*)senderAddr, senderAddrLen);
    if (bytes_recvd <= 0)
        perror("Reception socket reception");

    close(socket_fd);

cleanup:
    // TODO wrap addrinfo in a class
        // so it frees its memory on destruction
    freeaddrinfo(result);
    return bytes_recvd;
}


ssize_t UdpBroker::send(const char* ip, const char* port,
        const void* data, const size_t len)
{
    // Init struct to default values (brace --> value initialisation)
    struct addrinfo hints {};
    struct addrinfo* server_info = nullptr;

    // TODO make this selectable on call
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    int socket_fd      = 0;
    int gai_ret        = 0;
    ssize_t bytes_sent = -1;

    gai_ret = getaddrinfo(ip, port, &hints, &server_info);
    if (gai_ret != 0)
    {
        fprintf(stderr, "get addr info error: %s\n", gai_strerror(gai_ret));
        goto cleanup;
    }

    // WARN: Ignoring possibility of multiple address information
    // returns from getaddrinfo() in addrinfo struct

    errno = 0;
    socket_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (socket_fd == -1)
    {
        perror("Send socket creation");
        goto cleanup;
    }

    // NOTE: Stretch goal
    // Use `bind()` to allow sending from a specific IP
    // Useful if you have multiple interfaces, each with their own IP

    errno = 0;
    bytes_sent = sendto(socket_fd, data, len, 0,
            server_info->ai_addr, server_info->ai_addrlen);
    if (bytes_sent == -1)
        perror("Send socket sendto");

    close(socket_fd);

cleanup:
    // TODO wrap addrinfo in a class
        // so it frees its memory on destruction
    freeaddrinfo(server_info);
    perror("Send errored");
    return bytes_sent;
}


bool UdpBroker::sendDelayed(const char* ip, const char* port,
        const void* data, const size_t len, std::chrono::seconds delay)
{
    // NOTE: Stretch goal
    // Allow for any std::chrono::duration amount of delay
    // Be it us, ms, mins etc.

    // Ensure number is constrained
    // Alternatively, std::clamp() can be used
    if ((delay < 1s) || (delay > 255s))
        return false;

    std::string ip_mt   = ip;
    std::string port_mt = port;
    std::vector<uint8_t> data_mt {
            static_cast<const uint8_t*>(data),
            static_cast<const uint8_t*>(data) + len};

    _workers.emplace_back(
        // TODO bake in cond var waiting into the worker thread class
        // TODO possibly replace with timed_mutex and try_lock_for

        // Capture by value for ownership
        // Important in multithreaded contexts
        [this, delay, ip = std::move(ip_mt),
            port = std::move(port_mt), data = std::move(data_mt)]()
            {
                // TODO this functionality would be much nicer as a class
                // Wait using a condition variable, so when the
                // main object is destroyed, we end the thread
                std::unique_lock<std::mutex> lock(this->_workerMutex);
                this->_workerCondVar.wait_for(lock, delay);

                // If we're still allowed to execute
                // As cond var may have disturbed our slumber
                // to stop & get joined
                if (_workerExecFlag.load())
                {
                    UdpBroker::send(ip.data(), port.data(),
                            data.data(), data.size());
                }
            }
    );

    return true;
}


// TODO this shares VAST amounts of code with sendDelayed
// TODO combine the two to de-duplicate
bool UdpBroker::sendPeriodic(const char* ip, const char* port,
        const void* data, const size_t len, std::chrono::seconds interval)
{
    // Ensure number is constrained
    // Alternatively, std::clamp() can be used
    if ((interval < 1s) || (interval > 255s))
        return false;

    std::string ip_mt   = ip;
    std::string port_mt = port;
    std::vector<uint8_t> data_mt {
            static_cast<const uint8_t*>(data),
            static_cast<const uint8_t*>(data) + len};

    _workers.emplace_back(
        // TODO bake in cond var waiting into the worker thread class
        // TODO possibly replace with timed_mutex and try_lock_for

        // Capture by value for ownership
        // Important in multithreaded contexts
        [this, interval, ip = std::move(ip_mt),
            port = std::move(port_mt), data = std::move(data_mt)]()
            {
                std::unique_lock<std::mutex> lock(this->_workerMutex);

                // Continue while workers are alive
                while (_workerExecFlag.load())
                {
                    this->_workerCondVar.wait_for(lock, interval);

                    // If we're still allowed to execute
                    if (_workerExecFlag.load())
                    {
                        UdpBroker::send(ip.data(), port.data(),
                                data.data(), data.size());
                    }
                }
            }
    );

    return true;
}
