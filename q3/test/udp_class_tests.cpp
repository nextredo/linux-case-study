#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "udp_class.hpp"

TEST_SUITE("UDP")
{
    TEST_CASE("construction")
    {
        UdpSender();
    }
}
