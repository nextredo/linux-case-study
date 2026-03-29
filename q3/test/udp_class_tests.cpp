#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#include "doctest.h"
#include "udp_class.hpp"

#include <array>
#include <chrono>
#include <thread>
#include <future>

#include <unistd.h>
#include <sys/socket.h>

using namespace std::chrono_literals;
using namespace std::chrono;


// TODO instructions for pulling this repo
    // ensure submodule is pulled too & updated for q3 to work

// NOTE: Unit test limitations
// - Not tested on something other than loopback
// - Not tested with "well-known"/"system" port numbers (0 - 1024)

// TODO tests for
    // multiple senders at once
    // warn on port usage under 1024 (superuser only)
    // warn ports already in use
    // re-starting periodic send / delayed send during a current periodic / delayed send

namespace
{

// Maximum size to use for packet reception calls
constexpr size_t RX_DATA_LEN = 255;

// WARN: Will only work with std::chrono::duration types
// NOTE: Could constrain this with C++20 concepts
template<typename T>
auto fmt_as_s(T time)
{
    // NOTE: Use std::format in C++20 and above
    return duration<double, std::milli>(duration_cast<microseconds>(time)).count();
};

template<typename T>
void check_tolerance(seconds expected, T elapsed, int perc = 15)
{
    auto tol = [](seconds time, int perc)
    {
        auto time_ms = duration_cast<milliseconds>(time);
        return time_ms + ((time_ms / 100) * perc);
    };

    auto max = tol(expected, perc);
    auto min = tol(expected, -perc);

    INFO("took ",        fmt_as_s(elapsed), " ms");
    INFO("allowed max ", fmt_as_s(max), " ms");
    INFO("allowed min ", fmt_as_s(min), " ms");
    CHECK(elapsed < max);
    CHECK(elapsed > min);
}

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
        // Set default test variables
        UdpBroker::ip_ver_e ip_ver = UdpBroker::ip_ver_e::IPV4;
        const char* dst_ip         = "127.0.0.1";
        const char* dst_port       = "12345";
        const char* msg            = "Hello World!";

        bool expect_send_success = false;
        std::function<void()> rx_fn;

        SUBCASE("addresses")
        {
            SUBCASE("IPv4")
            {
                ip_ver = UdpBroker::ip_ver_e::IPV4;
                SUBCASE("alt_loopback") dst_ip = "127.0.0.2";
            }
            SUBCASE("IPv6")
            {
                ip_ver = UdpBroker::ip_ver_e::IPV6;

                // Only 1 subcase, IPv6 only has 1 loopback address
                // In contrast, IPv4 has all addrs in 127.0.0.0/8
                SUBCASE("loopback") dst_ip = "::1";
            }
        }

        SUBCASE("ports")
        {
            SUBCASE("ephemeral_1") dst_port = "12345";
            SUBCASE("ephemeral_2") dst_port = "55555";
        }

        SUBCASE("msgs")
        {
            SUBCASE("alt_msg") msg = "Hi Earth!!!";
        }

        // -------------------------- Test setup ---------------------------
        // Begin listening for (receiving/rx-ing) a packet
        // Using std::async over std::thread as it captures and stores any
        // exceptions thrown in the worker thread, re-throwing them in the main thread
        // once .get() is called on the corresponding std::future
        auto rx_future = std::async(std::launch::async,
            [dst_port, msg, dst_ip, ip_ver]()
            {
                // Reception will not work if packet is fragmented
                REQUIRE(std::strlen(msg) + 1 < RX_DATA_LEN);

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
        UdpBroker::ip_ver_e ip_ver = UdpBroker::ip_ver_e::IPV4;
        const char* dst_ip         = "127.0.0.1";
        const char* dst_port       = "56789";
        const char* msg            = "delayed send pkt contents";

        seconds delay = 1s;

        UdpBroker sender;
        bool      expect_send_success = false;

        std::function<void()> rx_fn;

        SUBCASE("out_of_range")
        {
            expect_send_success = false;
            SUBCASE("underrange") delay = 0s;
            SUBCASE("overrange")  delay = 256s;

            // Expect it to fail, so we don't need to check returns
            // For the paranoid, we could check that it doesn't send a packet
            // even after the specified delay
            rx_fn = []() {};
        }

        SUBCASE("in_range")
        {
            expect_send_success = true;
            SUBCASE("after_1s") delay = 1s;
            SUBCASE("after_2s") delay = 2s;
            SUBCASE("after_5s") delay = 5s;
            // SUBCASE("after_255s") delay = 255s;

            rx_fn = [dst_port, msg, dst_ip, ip_ver, delay]()
                {
                    // Wait for a little longer than the expected packet send delay
                    // WARN: API states we must only be able to delay for an integer number of seconds
                    auto rx_timeout = delay + 1s;

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

                    // Perform time checks & logging
                    auto time_to_rx = end - start;
                    check_tolerance(delay, time_to_rx);
                };
        }

        // -------------------------- Test setup ---------------------------
        // Begin async func to check for packet reception (or not)
        auto rx_future = std::async(std::launch::async, rx_fn);

        // Give receiver thread a headstart to begin listening
        std::this_thread::sleep_for(100ms);

        // Run the test
        bool queued_send = sender.sendDelayed(dst_ip, dst_port,
                (const uint8_t*)msg, std::strlen(msg), delay);

        // Wait for the receiver task to finish up
        rx_future.get();

        // Check function returned as expected
        CHECK(queued_send == expect_send_success);
    }

    TEST_CASE("sendPeriodic")
    {
        UdpBroker::ip_ver_e ip_ver = UdpBroker::ip_ver_e::IPV4;
        const char* dst_ip         = "127.0.0.1";
        const char* dst_port       = "57474";
        const char* msg            = "periodic send pkt contents";

        seconds interval         = 0s;
        size_t  interval_count   = 0;

        UdpBroker sender;
        bool      expect_send_success = false;

        std::function<void()> rx_fn;

        SUBCASE("out_of_range")
        {
            expect_send_success = false;
            SUBCASE("underrange") interval = 0s;
            SUBCASE("overrange")  interval = 256s;

            // Expect it to fail, so we don't need to check returns
            rx_fn = []() {};
        }

        SUBCASE("valid")
        {
            SUBCASE("1_packet")
            {
                expect_send_success = true;
                interval_count = 1;

                SUBCASE("short") interval = 2s;
                SUBCASE("long")  interval = 3s;
            }

            SUBCASE("3_packets")
            {
                expect_send_success = true;
                interval_count = 3;

                SUBCASE("short") interval = 2s;
                SUBCASE("long")  interval = 3s;
            }

            rx_fn = [dst_port, msg, dst_ip, ip_ver, interval, interval_count]()
                {
                    // Wait for a little longer than however many
                    // intervals we're testing (plus tolerance)
                    // WARN: API states we must only be able to delay for an integer number of seconds
                    auto rx_timeout = interval_count * interval + 1s;

                    uint8_t rx_data[RX_DATA_LEN] {};
                    struct sockaddr_storage sender_info {};
                    socklen_t sender_info_len = sizeof(sender_info);

                    // Loop for the receiving timeout period
                    size_t pkt_counter = 0;
                    auto loop_start = steady_clock::now();
                    while (steady_clock::now() - loop_start < rx_timeout)
                    {
                        auto start = steady_clock::now();
                        auto bytes_recvd = UdpBroker::recv(dst_port, rx_data, RX_DATA_LEN,
                                &sender_info, &sender_info_len, ip_ver, rx_timeout);
                        auto end = steady_clock::now();

                        if (bytes_recvd > 0)
                            ++pkt_counter;

                        // Cease checking if no bytes received
                        REQUIRE(bytes_recvd == std::strlen(msg));
                        CHECK((const char*)rx_data == msg);

                        // Perform time checks & logging
                        // Ensure each packet received approximately on-time
                        auto time_to_rx = end - start;
                        check_tolerance(interval, time_to_rx);
                    }

                    // Should have received 1 packet for each interval waited
                    CHECK(pkt_counter == interval_count);
                };
        }


        // -------------------------- Test setup ---------------------------
        // Begin async func to check for packet reception (or not)
        auto rx_future = std::async(std::launch::async, rx_fn);

        // Give receiver thread a headstart to begin listening
        std::this_thread::sleep_for(100ms);

        // Run the test
        bool queued_send = sender.sendPeriodic(dst_ip, dst_port,
                (const uint8_t*)msg, std::strlen(msg), interval);

        // Wait for the receiver task to finish up
        rx_future.get();

        // Check function returned as expected
        CHECK(queued_send == expect_send_success);
    }
}
