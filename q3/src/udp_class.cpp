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


// bool UdpBroker::recv()
// {
//     // Init struct to default values (brace --> value initialisation)
//     struct addrinfo hints {};
//     struct addrinfo* result = nullptr;
//
//     hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
//     hints.ai_socktype = SOCK_DGRAM; // UDP
//     hints.ai_flags = AI_PASSIVE; // Automatically fill in my IP
//     hints.ai_protocol = 0; // Any protocol
//
//     int socket_fd    = 0;
//     int gai_ret      = 0;
//     int bind_ret     = 0;
//     int listen_ret   = 0;
//     ssize_t recv_ret = 0;
//
//     int new_fd = 0;
//     struct sockaddr_storage their_addr {};
//     socklen_t their_addr_len {};
//
//     // int yes=1;
//     // setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
//
//     gai_ret = getaddrinfo(nullptr, port, &hints, &result);
//     if (gai_ret < 0)
//     {
//         fprintf(stderr, "get addr info error: %s\n", gai_strerror(gai_ret));
//         goto cleanup;
//     }
//
//     // TODO ???
//     // if (result->ai_next != nullptr)
//     // {
//     //     fprintf(stderr, "multiple addr infos\n");
//     //     goto cleanup;
//     // }
//
//     errno = 0;
//     socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
//     if (socket_fd == -1)
//         goto cleanup;
//
//     errno = 0;
//     bind_ret = bind(socket_fd, result->ai_addr, result->ai_addrlen);
//     if (bind_ret == -1)
//         goto cleanup;
//
//     errno = 0;
//     static constexpr auto NUM_CONNS = 1;
//     listen_ret = listen(socket_fd, NUM_CONNS);
//     if (listen_ret == -1)
//         goto cleanup;
//
//     // errno = 0;
//     // new_fd = accept(socket_fd,
//     //         (struct sockaddr*)&their_addr, &their_addr_len);
//     // if (listen_ret == -1)
//     //     goto cleanup;
//
//     // Alternatively, use sendto() and recvfrom() on a UDP socket
//     // when not calling connect() first
//
//     errno = 0;
//     recv_ret = ::recv(socket_fd, data, len, 0);
//     if (recv_ret <= 0)
//         goto cleanup;
//
//     // close(new_fd);
//     close(socket_fd);
//     return send_ret;
//
// cleanup:
//     freeaddrinfo(result);
//     perror("Errored");
//     return -1;
// }


ssize_t UdpBroker::send(struct sockaddr_storage* addr, const char* port, const uint8_t* data, size_t len)
{
    // Init struct to default values (brace --> value initialisation)
    struct addrinfo hints {};
    struct addrinfo* server_info = nullptr;

    // TODO make this selectable on call
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_addr = (struct sockaddr*)addr; // Destination address
    // hints.ai_protocol = 0; // Any protocol

    int socket_fd    = 0;
    int gai_ret      = 0;
    int connect_ret  = 0;
    ssize_t send_ret = -1;

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
    send_ret = sendto(socket_fd, data, len, 0,
            server_info->ai_addr, server_info->ai_addrlen);
    if (send_ret == -1)
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
    perror("Errored");
    return send_ret;
}


