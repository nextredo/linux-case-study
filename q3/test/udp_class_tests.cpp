#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "udp_class.hpp"

#include <unistd.h>
#include <sys/socket.h>

TEST_SUITE("UDP")
{
    TEST_CASE("object")
    {
        SUBCASE("construct_destruct") UdpSender();
        CHECK(true);
    }

    TEST_CASE("loopback")
    {
        struct sockaddr_storage dst_ip {};
        inet_pton(AF_INET,
                "127.0.0.1",
                &((struct sockaddr_in*)(&dst_ip))->sin_addr
        );
        const char* port = "55555";
        std::string msg = "hello world!!!!";

        UdpSender sender;

        SUBCASE("sendImmediate")
        {
            CHECK(sender.send(&dst_ip, port, (const uint8_t*)msg.data(), msg.size()) > 0);


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
