#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "udp_class.hpp"

#include <chrono>
#include <algorithm>
#include <iterator>
#include <thread>
#include <future>

#include <unistd.h>
#include <sys/socket.h>

using namespace std::chrono_literals;

namespace
{

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
        // Set test permutations
        constexpr const char* IPV4_DSTS[] = {"127.0.0.1",    "127.0.0.2"};
        constexpr const char* IPV6_DSTS[] = {"::1",          "::2"};
        constexpr const char* DST_PORTS[] = {"12345",        "55555"};
        constexpr const char* MSGS[]      = {"hello world!", "hi earth!"};

        // Set default test variables
        const char* dst_ip         = IPV4_DSTS[0];
        const char* dst_port       = DST_PORTS[0];

        UdpBroker::ip_ver_e ip_ver = UdpBroker::ip_ver_e::UNSPEC;
        std::string msg            = MSGS[0];

        SUBCASE("sendImmediate")
        {
            SUBCASE("addresses")
            {
                SUBCASE("IPv4")
                {
                    ip_ver = UdpBroker::ip_ver_e::IPV4;
                    SUBCASE(IPV4_DSTS[0]) dst_ip = IPV4_DSTS[0];
                    SUBCASE(IPV4_DSTS[1]) dst_ip = IPV4_DSTS[1];
                }
                SUBCASE("IPv6")
                {
                    ip_ver = UdpBroker::ip_ver_e::IPV6;

                    // Only 1 subcase, IPv6 only has 1 loopback address
                    // In contrast, IPv4 has all addrs in 127.0.0.0/8
                    SUBCASE(IPV6_DSTS[0]) dst_ip = IPV6_DSTS[0];
                }
            }

            SUBCASE("ports")
            {
                SUBCASE(DST_PORTS[0]) dst_port = DST_PORTS[0];
                SUBCASE(DST_PORTS[1]) dst_port = DST_PORTS[1];
            }

            SUBCASE("msgs")
            {
                SUBCASE(MSGS[0]) msg = MSGS[0];
                SUBCASE(MSGS[1]) msg = MSGS[1];
            }

            // -------------------------- Test setup ---------------------------
            // Begin listening for a packet
            // Using std::async over std::thread as it captures and stores any
            // exceptions thrown in the worker thread, re-throwing them in the main thread
            // once .get() is called on the corresponding std::future
            auto receiver_future = std::async(std::launch::async,
                [dst_port, msg, dst_ip, ip_ver]()
                {
                    // Listen for sent packet
                    constexpr size_t RX_DATA_LEN = 255;

                    // TODO std vector instead
                    uint8_t rx_data[RX_DATA_LEN] {};
                    struct sockaddr_storage sender_info {};
                    socklen_t sender_info_len = sizeof(sender_info);

                    // Blocking reception call
                    auto bytes_recvd = UdpBroker::recv(ip_ver, dst_port, rx_data, RX_DATA_LEN, &sender_info, &sender_info_len);

                    // Capture sender and destination IPs
                    CAPTURE(UdpBroker::decodeIp(&sender_info));
                    CAPTURE(std::string(dst_ip));
                    CAPTURE(std::string(dst_port));

                    // Check expected no. of bytes received
                    // Require as there's no point checking the msg contents if the length doesn't match
                    REQUIRE(bytes_recvd == std::size(msg));

                    // Check the packets contents match (only compare the received bytes, not entire array)
                    auto received_msg = std::string(std::begin(rx_data), std::begin(rx_data) + bytes_recvd);
                    CHECK(received_msg == msg);
                }
            );

            // Wait for reception thread to begin
            std::this_thread::sleep_for(1s);

            // Send the packet
            auto bytes_sent = UdpBroker::send(dst_ip, dst_port, (const uint8_t*)msg.data(), std::size(msg));

            // Complete receiver task
            // get() rethrows any captured exception from the thread
            receiver_future.get();

            // Check msg length matches
            // WARN: This assumption falls apart for pkts
            // longer than the allowed max UDP pkt length.
            // >0 should be sufficient
            CHECK(bytes_sent == std::size(msg));
        }

        SUBCASE("sendDelay")
        {
        }

        SUBCASE("sendPeriodic")
        {
        }
    }
}
