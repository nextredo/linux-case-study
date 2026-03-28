#include "udp_class.hpp"

std::string UdpBroker::ipNetworkToPresentation(struct addrinfo* addrInfo)
{
    std::string str;
    auto af = addrInfo->ai_family;

    switch (af)
    {
        case AF_INET:
            str.resize(INET_ADDRSTRLEN);
            break;

        case AF_INET6:
            str.resize(INET6_ADDRSTRLEN);
            break;

        default:
            break;
    };

    // TODO this sa_data cast will probably fail for IPv6
    inet_ntop(af, addrInfo->ai_addr->sa_data, str.data(), str.length());
    return str;
}

// bool UdpSender::sendImmediate(struct sockaddr*, uint16_t port, std::vector<uint8_t>& data)
// {
//     return true;
// }


// int yes=1;
// setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

ssize_t UdpBroker::send(struct sockaddr_storage* addr, const char* port, const uint8_t* data, size_t len)
{
    // Init struct to default values (brace --> value initialisation)
    struct addrinfo hints {};
    struct addrinfo* result = nullptr;

    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_addr = (struct sockaddr*)addr; // Destination address
    hints.ai_protocol = 0; // Any protocol

    int socket_fd    = 0;
    int gai_ret      = 0;
    int connect_ret  = 0;
    ssize_t send_ret = 0;

    gai_ret = getaddrinfo(nullptr, port, &hints, &result);
    if (gai_ret < 0)
    {
        fprintf(stderr, "get addr info error: %s\n", gai_strerror(gai_ret));
        goto cleanup;
    }

    // TODO ???
    // if (result->ai_next != nullptr)
    // {
    //     fprintf(stderr, "multiple addr infos\n");
    //     goto cleanup;
    // }

    errno = 0;
    socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (socket_fd == -1)
        goto cleanup;

    errno = 0;
    connect_ret = connect(socket_fd, result->ai_addr, result->ai_addrlen);
    if (connect_ret == -1)
        goto cleanup;

    // Alternatively, use sendto() and recvfrom() on a UDP socket
    // when not calling connect() first

    errno = 0;
    send_ret = ::send(socket_fd, data, len, 0);
    if (send_ret == -1)
        goto cleanup;

    close(socket_fd);
    return send_ret;

cleanup:
    freeaddrinfo(result);
    perror("Errored");
    return -1;
}


