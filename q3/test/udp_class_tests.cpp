#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "udp_class.hpp"

#include <thread>
#include <atomic>

#include <unistd.h>
#include <sys/socket.h>

namespace
{

bool encode_ip(const sa_family_t ipVer, const char* ip, struct sockaddr_storage* ipData)
{
    int ret = -1;
    void* ip_output = nullptr;

    switch (ipVer)
    {
        case AF_INET:
            ip_output = &((struct sockaddr_in*)(&ipData))->sin_addr;
            break;

        case AF_INET6:
            ip_output = &((struct sockaddr_in6*)(&ipData))->sin6_addr;
            break;

        default:
            FAIL(true);
            break;
    };

    ret = inet_pton(ipData->ss_family, ip, ip_output);
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
        const char* const port = "55555";
        std::string msg        = "hello world!!!!";

        sa_family_t ip_ver = AF_INET;
        const char* ip = "127.0.0.1";
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

            // Send the packet
            auto bytes_sent = UdpBroker::send(&dst_ip, port, (const uint8_t*)msg.data(), msg.size());

            // Listen for sent packet
            constexpr size_t        RX_DATA_LEN = 255;
            uint8_t                 rx_data[RX_DATA_LEN] {};
            struct sockaddr_storage rx_sender_info {};
            socklen_t               rx_sender_info_len {};

            // Blocking reception call
            // UdpBroker::recv(port, rx_data, RX_DATA_LEN, &rx_sender_info, &rx_sender_info_len);

            // Verify it reported success
            CHECK(bytes_sent > 0);

            // Check msg length matches
            // WARN: This assumption falls apart for pkts
            // longer than the allowed max UDP pkt length
            CHECK(bytes_sent == msg.size());

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
