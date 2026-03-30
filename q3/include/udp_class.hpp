// Include guard
// Alternatively, use `#pragma once`
#ifndef UDP_CLASS_H
#define UDP_CLASS_H

#include <cstddef>
#include <cerrno>
#include <cstring>

#include <atomic>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <string>

#include <unistd.h>
#include <netinet/ip.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>

// NOTE: Using declarations in header files is bad practice
using namespace std::chrono;
using namespace std::chrono_literals;


/// @brief Address information class
/// @note  Ensures @p addrinfo struct from POSIX is freed on destruction
class AddressInfo
{
private:
    struct addrinfo* _info   = nullptr; ///< Handle to address information
    int              _gaiRet = -1;      ///< Validity of address info

public:
    /// @name Basic getters
    /// @{
    [[nodiscard]] bool valid() const  { return (_gaiRet == 0); }
    [[nodiscard]] auto errStr() const { return gai_strerror(_gaiRet); }
    /// @}

    /// @brief access to underlying information
    [[nodiscard]] const auto* operator ->() { return _info; }

    /// @brief Deleted default constructor
    AddressInfo() = delete;

    /// @brief Address information constructor
    /// @param name    Host to get info for
    /// @param service Service to get info for
    /// @param hints   Hints to use while getting info
    AddressInfo(const char* name, const char* service,
            const struct addrinfo* hints) :
        _gaiRet(getaddrinfo(name, service, hints, &_info))
    { }

    /// @brief Freeing destructor
    ~AddressInfo() { freeaddrinfo(_info); }
};

// TODO doxygen - here, source, unit tests

/// @brief Basic send/receive class
/// @note Class limitations
/// @n    Doesn't handle packet fragmentation
/// @n    Doesn't allow multiple background workers at once
class UdpBroker
{
private:
    // TODO Bundle cond var, mutex, thread into a worker class
        // Pass reference to all-worker atomic exec flag on construction
    // TODO Implement support for multiple background workers
        // Vector of workers
        // Unit tests for operations involving multiple workers

    /// Signal to stop workers
    std::atomic<bool> _workerExecFlag = true;
    std::thread       _worker; ///< Used for background tasks

    std::mutex              _workerMutex;   ///< Cond var access ctrl
    std::condition_variable _workerCondVar; ///< Used for interruptible sleep

    /// @brief Stops background work
    void stopWorker();

    /// @brief Allows background work
    void allowWork() { _workerExecFlag = true; }

public:
    /// @brief Protocol types
    enum ip_ver_e : int
    {
        UNSPEC = AF_UNSPEC,
        IPV4   = AF_INET,
        IPV6   = AF_INET6,
    };

    /// @brief Destructor
    /// @note  Required for worker thread termination
    ~UdpBroker();

    /// @brief Convert struct holding IP info to a string
    /// @param  Socket address struct containing an IP
    /// @return Human-readable string
    static std::string decodeIp(struct sockaddr_storage* sa);

    /// @brief Sends a UDP packet (immediately)
    /// @return Number of bytes sent. Can be less than the input number
    /// if, for example, information is to be split across multiple packets
    /// @param ip   Destination IP
    /// @param port Host port to listen on
    /// @param data Buffer of data to send
    /// @param len  Length of the data buffer, in bytes
    /// @return Number of bytes sent
    static ssize_t send(const char* ip, const char* port,
            const void* data, const size_t len);

    /// @brief Receives a UDP packet
    /// @param      port          Host port to listen on
    /// @param[out] data          Buffer to put received data in
    /// @param      len           Length of the data buffer, in bytes
    /// @param[out] senderAddr    Info about the sender
    /// @param[out] senderAddrLen Length of the received sender info
    /// @param      ipVer         Which IP version to listen on
    /// @param      timeout       How long to wait for a packet
    /// @return Number of bytes received
    static ssize_t recv(const char* port, void* data, const size_t len,
            struct sockaddr_storage* senderAddr, socklen_t* senderAddrLen,
            const ip_ver_e ipVer = ip_ver_e::UNSPEC,
            const microseconds timeout = 3s);

    /// @brief Sends a UDP packet after a specified delay
    /// @param ip    Destination IP
    /// @param port  Host port to listen on
    /// @param data  Buffer of data to send
    /// @param delay How long to wait to send a packet
    /// @return @p true if send successfully queued. @p false if not.
    bool sendDelayed(const char* ip, const char* port,
        const void* data, const size_t len,
        const seconds delay);

    /// @brief   Sends a UDP packet periodically, forever
    /// @details Sends the first packet instantly
    /// @param ip       Destination IP
    /// @param port     Host port to listen on
    /// @param data     Buffer of data to send
    /// @param interval How long to wait between pkt sends
    /// @return @p true if send successfully queued. @p false if not.
    bool sendPeriodic(const char* ip, const char* port,
        const void* data, const size_t len,
        const seconds interval);
};


#endif // UDP_CLASS_H
