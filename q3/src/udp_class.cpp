#include "udp_class.hpp"

std::string UdpBroker::decodeIp(struct sockaddr_storage* sa)
{
    std::string str;
    auto af = sa->ss_family;
    void* ip = nullptr;

    // TODO refactor decode and encode ip to share the pointer casting stuff
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

// TODO refactor so it doesn't need ipVer parameter
bool UdpBroker::encodeIp(const sa_family_t ipVer, const char* ip, struct sockaddr_storage* ipData)
{
    int ret = -1;
    void* ip_output = nullptr;

    switch (ipVer)
    {
        case AF_INET:
            ip_output = &((struct sockaddr_in*)ipData)->sin_addr;
            break;

        case AF_INET6:
            ip_output = &((struct sockaddr_in6*)ipData)->sin6_addr;
            break;

        default:
            break;
    };

    ret = inet_pton(ipVer, ip, ip_output);
    return (ret == 1);
}

// TODO allow socket re-binding without errors
// int yes=1;
// char yes='1'; // Solaris people use this
// lose the pesky "Address already in use" error message
// setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

ssize_t UdpBroker::recv(const char* port, uint8_t* data, const size_t len, struct sockaddr_storage* senderAddr, socklen_t* senderAddrLen)
{
    // Init struct to default values (brace --> value initialisation)
    struct addrinfo hints {};
    struct addrinfo* result = nullptr;

    // TODO make this selectable on call (enum class)
    // TODO combine with send() setup
    hints.ai_family   = AF_UNSPEC;   // Use any address family (IPv4 or IPv6 here)
    hints.ai_socktype = SOCK_DGRAM;  // Only return datagram type sockets
    hints.ai_flags    = AI_PASSIVE;  // Autofill my IP socket (used for incoming connections)
    hints.ai_protocol = IPPROTO_UDP; // Only return sockets for the UDP protocol

    int socket_fd       = 0;
    int gai_ret         = 0;
    int bind_ret        = 0;
    int sockopt_ret     = 0;
    int listen_ret      = 0;
    ssize_t bytes_recvd = -1;

    timeval sock_timeout = { .tv_sec = 3 };

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


ssize_t UdpBroker::send(const char* ip, const char* port, const uint8_t* data, const size_t len)
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

    // TODO insert `bind` function to allow sending from a specific IP

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


