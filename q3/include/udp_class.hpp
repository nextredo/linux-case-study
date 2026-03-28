#include <cstdint>
#include <cerrno>

#include <vector>
#include <string>

#include <unistd.h>
#include <netinet/ip.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>

// Include guard
// Alternatively, use `#pragma once`
#ifndef UDP_SENDER_H
#define UDP_SENDER_H

// TODO warn on port usage under 1024 (superuser only)
// TODO warn ports already in use
// TODO set send() recv() timeouts with setsockopt()
// TODO doxygen
// TODO errno usage
// TODO debug function to print dest ip (inet_ntop INET_ADDRSTRLEN)

class UdpBroker
{
private:
    static constexpr auto BUF_LEN = 1024;

    std::string ipNetworkToPresentation(struct addrinfo* addrInfo);

public:
    /// @brief Sends a UDP packet
    ssize_t send(struct sockaddr_storage* addr, const char* port, const uint8_t* data, size_t len);

    /// @brief Sends a UDP packet instantly
    bool sendImmediate(struct in_addr* ip, uint16_t port, std::vector<uint8_t>& data);

    /// @brief Sends a UDP packet after a specified delay
    bool sendDelay();

    /// @brief Sends a UDP packet periodically
    bool sendPeriodic();
};


#endif // UDP_SENDER_H
