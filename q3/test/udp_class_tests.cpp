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
        uint16_t dst_port = 3;
        struct in_addr dst_ip {};
        inet_aton("127.0.0.1", &dst_ip);
        std::vector<uint8_t> data {1, 2, 3, 4};

        UdpSender sender;

        SUBCASE("sendImmediate")
        {
            CHECK(sender.sendImmediate(&dst_ip, dst_port, data));
            // TODO check pkt actuall received
        }

        CHECK(true);
    }
}
