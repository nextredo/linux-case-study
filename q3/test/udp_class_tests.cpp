#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
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

// Maximum size to use for packet reception calls
constexpr size_t RX_DATA_LEN = 255;

// TODO remove, duplicates 1st lambda
// void expect_packet(UdpBroker::ip_ver_e ip_ver, const char* dst_ip,
//         const char* dst_port, const char *msg)
// {
//     // Listen for sent packet
//     constexpr size_t RX_DATA_LEN = 255;
//
//     // TODO std vector instead
//     uint8_t rx_data[RX_DATA_LEN] {};
//     struct sockaddr_storage sender_info {};
//     socklen_t sender_info_len = sizeof(sender_info);
//
//     // Blocking reception call
//     auto bytes_recvd = UdpBroker::recv(dst_port, rx_data, RX_DATA_LEN, &sender_info, &sender_info_len, ip_ver);
//
//     // Capture sender and destination IPs
//     CAPTURE(UdpBroker::decodeIp(&sender_info));
//     CAPTURE(dst_ip);
//     CAPTURE(dst_port);
//
//     // Check expected no. of bytes received
//     // Require as there's no point checking the msg contents if the length doesn't match
//     REQUIRE(bytes_recvd == std::strlen(msg));
//
//     // Check the packets contents match (only compare the received bytes, not entire array)
//     CHECK((const char*)rx_data == msg);
// }

}

TEST_SUITE("UDP")
{
    TEST_CASE("object")
    {
        SUBCASE("construct_destruct") UdpBroker();

        CHECK(true);
    }

    TEST_CASE("send")
    {
        // Set test permutations
        constexpr const char* IPV4_DSTS[] = {"127.0.0.1",    "127.0.0.2"};
        constexpr const char* IPV6_DSTS[] = {"::1",          "::2"};
        constexpr const char* DST_PORTS[] = {"12345",        "55555"};
        constexpr const char* MSGS[]      = {"hello world!", "hi earth!"};

        // Set default test variables
        UdpBroker::ip_ver_e ip_ver = UdpBroker::ip_ver_e::IPV4;
        const char* dst_ip         = IPV4_DSTS[0];
        const char* dst_port       = DST_PORTS[0];
        const char* msg            = MSGS[0];


        // SUBCASE("addresses")
        // {
        //     SUBCASE("IPv4")
        //     {
        //         ip_ver = UdpBroker::ip_ver_e::IPV4;
        //         SUBCASE(IPV4_DSTS[0]) dst_ip = IPV4_DSTS[0];
        //         SUBCASE(IPV4_DSTS[1]) dst_ip = IPV4_DSTS[1];
        //     }
        //     SUBCASE("IPv6")
        //     {
        //         ip_ver = UdpBroker::ip_ver_e::IPV6;
        //
        //         // Only 1 subcase, IPv6 only has 1 loopback address
        //         // In contrast, IPv4 has all addrs in 127.0.0.0/8
        //         SUBCASE(IPV6_DSTS[0]) dst_ip = IPV6_DSTS[0];
        //     }
        // }
        //
        // SUBCASE("ports")
        // {
        //     SUBCASE(DST_PORTS[0]) dst_port = DST_PORTS[0];
        //     SUBCASE(DST_PORTS[1]) dst_port = DST_PORTS[1];
        // }
        //
        // SUBCASE("msgs")
        // {
        //     SUBCASE(MSGS[0]) msg = MSGS[0];
        //     SUBCASE(MSGS[1]) msg = MSGS[1];
        // }

        // Reception will not work if packet is fragmented
        REQUIRE(std::strlen(msg) + 1 < RX_DATA_LEN);

        // -------------------------- Test setup ---------------------------
        // Begin listening for (receiving/rx-ing) a packet
        // Using std::async over std::thread as it captures and stores any
        // exceptions thrown in the worker thread, re-throwing them in the main thread
        // once .get() is called on the corresponding std::future
        auto rx_future = std::async(std::launch::async,
            [dst_port, msg, dst_ip, ip_ver]()
            {
                // TODO std vector instead
                uint8_t rx_data[RX_DATA_LEN] {};
                struct sockaddr_storage sender_info {};
                socklen_t sender_info_len = sizeof(sender_info);

                // Blocking reception call
                auto bytes_recvd = UdpBroker::recv(dst_port, rx_data, RX_DATA_LEN, &sender_info, &sender_info_len, ip_ver);

                // Capture sender and destination IPs
                CAPTURE(UdpBroker::decodeIp(&sender_info));
                CAPTURE(dst_ip);
                CAPTURE(dst_port);

                // Check expected no. of bytes received
                // Require as there's no point checking the msg contents if the length doesn't match
                REQUIRE(bytes_recvd == std::strlen(msg));

                // Check the packets contents match (only compare the received bytes, not entire array)
                CHECK((const char*)rx_data == msg);
            }
        );

        // Wait for reception thread to begin
        std::this_thread::sleep_for(100ms);

        // Send the packet
        // TODO fix this not sending the null byte of the string
            // change msg to be stored as a const void* and actual length instead
        auto bytes_sent = UdpBroker::send(dst_ip, dst_port, (const uint8_t*)msg, std::strlen(msg));

        // Complete receiver task
        // get() rethrows any captured exception from the thread
        rx_future.get();

        // Check msg length matches
        // WARN: This assumption falls apart for pkts
        // longer than the allowed max UDP pkt length.
        // >0 should be sufficient
        CHECK(bytes_sent == std::strlen(msg));
    }

    TEST_CASE("sendDelayed")
    {
        using namespace std::chrono;

        UdpBroker::ip_ver_e ip_ver = UdpBroker::ip_ver_e::IPV4;
        const char* dst_ip         = "127.0.0.1";
        const char* dst_port       = "56789";
        const char* msg            = "delayed send pkt contents";

        std::chrono::seconds delay = 0s;

        UdpBroker sender;
        bool      expect_success = false;

        std::function<void()> rx_fn;

        SUBCASE("out_of_range")
        {
            expect_success = false;
            SUBCASE("after_0s") delay = 0s;
            SUBCASE("after_256s") delay = 256s;

            // Expect it to fail, so we don't need to check returns
            // For the paranoid, we could check that it doesn't send a packet even
            // after the specified delay
            rx_fn = []() {};
        }

        SUBCASE("in_range")
        {
            expect_success = true;
            SUBCASE("after_1s") delay = 1s;
            SUBCASE("after_2s") delay = 2s;
            SUBCASE("after_5s") delay = 5s;
            // SUBCASE("after_255s") delay = 255s;

            rx_fn = [dst_port, msg, dst_ip, ip_ver, delay]()
                {
                    // Wait for a little longer than the expected packet send delay
                    auto rx_timeout = delay + 2s;
                    auto min_expected_time_to_rx = delay - 1s;
                    auto max_expected_time_to_rx = delay + 1s;

                    uint8_t rx_data[RX_DATA_LEN] {};
                    struct sockaddr_storage sender_info {};
                    socklen_t sender_info_len = sizeof(sender_info);

                    // Time the blocking reception call
                    // Ensure it waits for the delay period specified
                    // This is sufficient if we give the sender a headstart
                    // TODO this ideally shouldn't include socket setup, so as to make
                    // the time spent in this function almost all spent waiting on recvfrom() syscall
                    auto start = steady_clock::now();
                    auto bytes_recvd = UdpBroker::recv(dst_port, rx_data, RX_DATA_LEN,
                            &sender_info, &sender_info_len, ip_ver, rx_timeout);
                    auto end = steady_clock::now();

                    // Cease checking if no bytes received
                    REQUIRE(bytes_recvd == std::strlen(msg));
                    CHECK((const char*)rx_data == msg);

                    // NOTE: Use std::format in C++20 and above
                    auto fmt_as_s = [](auto t)
                    {
                        return duration<double, std::milli>(duration_cast<microseconds>(t)).count();
                    };

                    // Perform time checks & logging
                    auto time_to_rx = end - start;
                    INFO("took ",        fmt_as_s(time_to_rx), " ms");
                    INFO("allowed max ", fmt_as_s(max_expected_time_to_rx), " ms");
                    INFO("allowed min ", fmt_as_s(min_expected_time_to_rx), " ms");
                    CHECK(time_to_rx > min_expected_time_to_rx);
                    CHECK(time_to_rx < max_expected_time_to_rx);
                };
        }

        // Expect a packet to be received
        auto rx_future = std::async(std::launch::async, rx_fn);

        // Wait for reception thread to begin
        std::this_thread::sleep_for(100ms);

        // Run the test
        bool queued_send = sender.sendDelayed(dst_ip, dst_port,
                (const uint8_t*)msg, std::strlen(msg), delay);

        // Wait for queued send to complete
        if (expect_success)
            std::this_thread::sleep_for(delay);

        // Complete receiver task
        rx_future.get();

        // Check function returned as expected
        CHECK(queued_send == expect_success);
    }

    TEST_CASE("sendPeriodic")
    {
    }
}
