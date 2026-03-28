#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "udp_class.hpp"

#include <unistd.h>
#include <sys/socket.h>

TEST_SUITE("UDP")
{
    TEST_CASE("object")
    {
        SUBCASE("construct_destruct") UdpBroker();
        CHECK(true);
    }

    TEST_CASE("loopback")
    {
        const char* ip   = "127.0.0.1";
        const char* port = "55555";
        std::string msg  = "hello world!!!!";

        UdpBroker sender;

        SUBCASE("sendImmediate")
        {
            SUBCASE("IPv4")
            {
                constexpr const char* IPV4_ADDRS[] = {
                    "127.0.0.2", "127.0.0.3",
                };
                SUBCASE(IPV4_ADDRS[0]) ip = IPV4_ADDRS[0];
                SUBCASE(IPV4_ADDRS[1]) ip = IPV4_ADDRS[1];
            }
            SUBCASE("IPv6")
            {
                constexpr const char* IPV6_ADDRS[] = {
                    "::1", "::2",
                };
                SUBCASE(IPV6_ADDRS[0]) ip = IPV6_ADDRS[0];
                SUBCASE(IPV6_ADDRS[1]) ip = IPV6_ADDRS[1];
            }
            SUBCASE("port")
            {
                constexpr const char* PORTS[] = {
                    "12345", "9876",
                };
                SUBCASE(PORTS[0]) port = PORTS[0];
                SUBCASE(PORTS[1]) port = PORTS[1];
            }

            struct sockaddr_storage dst_ip {};
            inet_pton(AF_INET, ip,
                    &((struct sockaddr_in*)(&dst_ip))->sin_addr);

            auto bytes_sent = sender.send(&dst_ip, port,
                    (const uint8_t*)msg.data(), msg.size());

            CHECK(bytes_sent > 0);


            // TODO check pkt actually received
                // wireshark, or via C
        }

        SUBCASE("sendDelay")
        {
        }

        SUBCASE("sendPeriodic")
        {
        }

        CHECK(true);
    }
}
