#include <cstdint>
#include <cerrno>
#include <cstring>

#include <vector>
#include <string>

#include <unistd.h>
#include <netinet/ip.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>

// Include guard
// Alternatively, use `#pragma once`
#ifndef UDP_CLASS_H
#define UDP_CLASS_H

// TODO warn on port usage under 1024 (superuser only)
// TODO warn ports already in use
// TODO set send() recv() timeouts with setsockopt()
// TODO doxygen - here, source, unit tests
// TODO errno usage
// TODO debug function to print dest ip (inet_ntop INET_ADDRSTRLEN)

class UdpSender
{
private:
public:
};

class UdpReceiver
{
private:
public:
};

class UdpBroker
{
private:

public:
    enum ip_ver_e : int
    {
        UNSPEC = AF_UNSPEC,
        IPV4   = AF_INET,
        IPV6   = AF_INET6,
    };

    static std::string decodeIp(struct sockaddr_storage* sa);

    static bool encodeIp(const char* ip, struct sockaddr_storage* ipData);

    /// @brief Sends a UDP packet
    /// @return Number of bytes sent. Can be less than the input number
    /// if, for example, information is to be split across multiple packets
    static ssize_t send(const char* ip, const char* port, const uint8_t* data, const size_t len);

    /// @brief Receives a UDP packet
    /// @param      port           Host port to listen on
    /// @param[out] data           Buffer to put received data in
    /// @param      len            Length of the data buffer
    /// @param[out] senderAddr     Info about the sender
    /// @param[out] senderAddrLen  Length of the received sender info
    /// @param      ip_ver_e       Which IP protocol to listen on
    /// @return Number of bytes received
    static ssize_t recv(const char* port, uint8_t* data, const size_t len, struct sockaddr_storage* senderAddr, socklen_t* senderAddrLen, const ip_ver_e ipVer = ip_ver_e::UNSPEC);

    /// @brief Sends a UDP packet instantly
    bool sendImmediate();

    /// @brief Sends a UDP packet after a specified delay
    bool sendDelay();

    /// @brief Sends a UDP packet periodically
    bool sendPeriodic();

    static std::string ipNetworkToPresentation(struct addrinfo* addrInfo);
    static void ipPresentationToNetwork();
};


#endif // UDP_CLASS_H
