#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "udp_class.hpp"

#include <chrono>
#include <thread>
#include <algorithm>
#include <iterator>

#include <unistd.h>
#include <sys/socket.h>

using namespace std::chrono_literals;

namespace
{

std::string decode_ip(struct sockaddr_storage* sa)
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

bool encode_ip(const sa_family_t ipVer, const char* ip, struct sockaddr_storage* ipData)
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
            FAIL(true);
            break;
    };

    ret = inet_pton(ipVer, ip, ip_output);
    return (ret == 1);
}

}

TEST_SUITE("UDP")
{
    TEST_CASE("object")
    {
        SUBCASE("construct_destruct") UdpBroker();

        CHECK(true);
    }

    TEST_CASE("loopback")
    {
        constexpr char port[] = "55555";
        constexpr char msg[]  = "hello world!!!!";
        constexpr char ip[]   = "127.0.0.1";

        sa_family_t ip_ver = AF_INET;
        struct sockaddr_storage dst_ip {};
        encode_ip(ip_ver, ip, &dst_ip);

        SUBCASE("sendImmediate")
        {
            // SUBCASE("addresses")
            // {
            //     SUBCASE("IPv4")
            //     {
            //         constexpr const char* IPV4_ADDRS[] = {
            //             "127.0.0.2", "127.0.0.3",
            //         };
            //         SUBCASE(IPV4_ADDRS[0]) ip = IPV4_ADDRS[0];
            //         SUBCASE(IPV4_ADDRS[1]) ip = IPV4_ADDRS[1];
            //     }
            //     SUBCASE("IPv6")
            //     {
            //         ip_ver = AF_INET6;
            //         constexpr const char* IPV6_ADDRS[] = {
            //             "::1", "::2",
            //         };
            //         SUBCASE(IPV6_ADDRS[0]) ip = IPV6_ADDRS[0];
            //         SUBCASE(IPV6_ADDRS[1]) ip = IPV6_ADDRS[1];
            //     }
            // }
            //
            // SUBCASE("port")
            // {
            //     constexpr const char* PORTS[] = {
            //         "12345", "9876",
            //     };
            //     SUBCASE(PORTS[0]) port = PORTS[0];
            //     SUBCASE(PORTS[1]) port = PORTS[1];
            // }

            // --- test setup ---
            // Setup the sockaddr object
            encode_ip(ip_ver, ip, &dst_ip);

            // Begin listening for a packet
            std::thread receiver(
                [port, msg, ip]()
                {
                    // Listen for sent packet
                    constexpr size_t        RX_DATA_LEN = 255;

                    // TODO std vector instead
                    uint8_t rx_data[RX_DATA_LEN] {};
                    struct sockaddr_storage rx_sender_info {};
                    socklen_t rx_sender_info_len = sizeof(rx_sender_info);

                    // Blocking reception call
                    auto bytes_recvd = UdpBroker::recv(port, rx_data, RX_DATA_LEN, &rx_sender_info, &rx_sender_info_len);

                    // Check expected no. of bytes received
                    CHECK(bytes_recvd == std::size(msg));

                    // Check sender is as expected
                    CHECK(decode_ip(&rx_sender_info) == std::string(ip));

                    // Check the packets contents match (only compare the received bytes, not entire array)
                    CAPTURE(std::string(std::begin(rx_data), std::begin(rx_data) + bytes_recvd));
                    CAPTURE(std::string(std::begin(msg),     std::end(msg)));

                    CHECK(std::equal(std::begin(rx_data), std::begin(rx_data) + bytes_recvd,
                                     std::begin(msg), std::end(msg)));
                }
            );

            // Wait for reception thread to begin
            std::this_thread::sleep_for(1s);

            // Send the packet
            auto bytes_sent = UdpBroker::send(&dst_ip, port, (const uint8_t*)msg, std::size(msg));

            // Complete receiver task
            receiver.join();

            // Check msg length matches
            // WARN: This assumption falls apart for pkts
            // longer than the allowed max UDP pkt length.
            // >0 should be sufficient
            CHECK(bytes_sent == std::size(msg));

            // TODO
            // Check reception
            // CHECK(broker.bytesReceived() == msg.size());
            // CHECK(broker.recv() == msg);


            // TODO check pkt actually received
                // wireshark, or via C
        }

        SUBCASE("sendDelay")
        {
        }

        SUBCASE("sendPeriodic")
        {
        }
    }
}
