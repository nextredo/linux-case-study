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


ssize_t UdpBroker::recv(const char* port, uint8_t* data, const size_t len, struct sockaddr_storage* senderAddr, socklen_t* senderAddrLen)
{
    // Init struct to default values (brace --> value initialisation)
    struct addrinfo hints {};
    struct addrinfo* result = nullptr;

    // TODO make this selectable on call
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_flags = AI_PASSIVE; // Automatically fill in my IP
    hints.ai_protocol = 0; // Any protocol

    int socket_fd       = 0;
    int gai_ret         = 0;
    int bind_ret        = 0;
    int listen_ret      = 0;
    ssize_t bytes_recvd = -1;

    // int yes=1;
    // setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    gai_ret = getaddrinfo(nullptr, port, &hints, &result);
    if (gai_ret < 0)
    {
        fprintf(stderr, "get addr info error: %s\n", gai_strerror(gai_ret));
        goto cleanup;
    }

    // WARN: Ignoring possibility of multiple address information
    // returns from getaddrinfo() in addrinfo struct

    errno = 0;
    socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (socket_fd == -1)
        goto cleanup;

    errno = 0;
    bind_ret = bind(socket_fd, result->ai_addr, result->ai_addrlen);
    if (bind_ret == -1)
        goto cleanup;

    errno = 0;
    bytes_recvd = recvfrom(socket_fd, data, len, 0,
            (struct sockaddr*)senderAddr, senderAddrLen);
    if (bytes_recvd <= 0)
        goto cleanup;

    close(socket_fd);

cleanup:
    // TODO wrap addrinfo in a class
        // so it frees its memory on destruction
    freeaddrinfo(result);
    // perror("Errored");
    return bytes_recvd;
}


ssize_t UdpBroker::send(const struct sockaddr_storage* addr, const char* port, const uint8_t* data, const size_t len)
{
    // Init struct to default values (brace --> value initialisation)
    struct addrinfo hints {};
    struct addrinfo* server_info = nullptr;

    // TODO make this selectable on call
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_addr = (struct sockaddr*)addr; // Destination address
    // hints.ai_protocol = 0; // Any protocol

    int socket_fd      = 0;
    int gai_ret        = 0;
    int connect_ret    = 0;
    ssize_t bytes_sent = -1;

    gai_ret = getaddrinfo(nullptr, port, &hints, &server_info);
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
        goto cleanup;

    errno = 0;
    bytes_sent = sendto(socket_fd, data, len, 0,
            server_info->ai_addr, server_info->ai_addrlen);
    if (bytes_sent == -1)
        goto cleanup;

    // Alternatively, can use connect() then just send() and recv()
    // for datagram communications
    // Alternatively, use sendto() and recvfrom() on a UDP socket
    // when not calling connect() first

    close(socket_fd);

cleanup:
    // TODO wrap addrinfo in a class
        // so it frees its memory on destruction
    freeaddrinfo(server_info);
    // perror("Errored");
    return bytes_sent;
}


