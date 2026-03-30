#include <chrono>
#include <atomic>
#include <thread>
#include <functional>
#include <iostream>

using namespace std::chrono_literals;
using namespace std::chrono;

// Translation unit local
namespace
{

void StartThread(
    std::thread& thread,
    std::atomic<bool>* running, // Take this as a pointer, not a reference
    const std::function<bool(void)>& process,
    const std::chrono::seconds timeout)
{
    thread = std::thread(
        // Capture by value, not by reference
        //
        // This removes the issue wherein the thead object has dangling references.
        // `StartThread` returns immediately after creating the thread object.
        // The thread object has ownership of the lambda given to it during construction,
        // however, the lambda had references to reference parameter which *only exist*
        // during the scope of `StartThread`.
        // As such, the lambda (and thus thread object) now hold dangling references
        [=]()
        {
            auto start = high_resolution_clock::now();
            while (*running)
            {
                bool aborted = process();

                auto end = high_resolution_clock::now();
                auto duration = duration_cast<milliseconds>(end - start);
                if (aborted || duration > timeout)
                {
                    *running = false;
                    break;
                }
            }
        });
}

}

int main()
{
    // Give each thread an execution flag variable, so they
    // do not interfere with each others' execution state
    std::atomic<bool> my_running1 = true;
    std::atomic<bool> my_running2 = true;

    std::thread my_thread1;
    std::thread my_thread2;

    // WARN: If these are accessed in multiple threads,
    // while >1 thread unjoined, they should be made atomic
    int loop_counter1 = 0;
    int loop_counter2 = 0;

    // Start actions in separate threads and wait for them
    StartThread(
        my_thread1,
        &my_running1,
        [&]()
        {
            // "some actions" simulated with waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            loop_counter1++;
            return false;
        },
        10s);

    StartThread(
        my_thread2,
        &my_running2,
        [&]()
        {
            // "some actions" simulated with waiting
            if (loop_counter2 < 5)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                loop_counter2++;
                return false;
            }
            return true;
        },
        10s);

    // Wait on thread execution
    my_thread1.join();
    my_thread2.join();

    // Print execlution loop counters
    std::cout << "C1: " << loop_counter1 << "\n";
    std::cout << "C2: " << loop_counter2 << "\n";
}
