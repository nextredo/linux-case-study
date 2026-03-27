#include <vector>
#include <cstdint>

#include <netinet/ip.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

// Include guard
// Alternatively, use `#pragma once`
#ifndef UDP_SENDER_H
#define UDP_SENDER_H

class UdpSender
{
public:
    bool sendImmediate(struct in_addr* ip, uint16_t port, std::vector<uint8_t>& data);
};


#endif // UDP_SENDER_H
