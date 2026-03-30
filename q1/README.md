# Question 1 - `cpp_task.cpp`
> Take a look at `cpp_task.cpp`<br>
> What is the code doing, are there any issues, and if so how to fix them?<br>

## Notes
### Files
- Original question in [`cpp_task.cpp`](./cpp_task.cpp)
  - Marked as readonly to prevent accidental modification
- Changed version in [`cpp_task_modified.cpp`](./cpp_task_modified.cpp)

```bash
# See changes between the files with;
diff cpp_task.cpp cpp_task_modified.cpp
```

## Running
```bash
# Compile & run the solution
g++ --std=c++17 cpp_task_modified.cpp && ./a.out
```

## Thought Process
### Initial Code Readthrough
- Nothing obviously wrong at first glance
- Appears to be C++17 code (uses `chrono_literals`)

### Initial Run
- Assume fairly standard `g++` compilation
  - `g++ --std=c++17 -Wall -Wextra -pedantic cpp_task.cpp`
- Output:
  - `C1:0 C2: 6`
- Expected
  - `loop_counter1` = 4
    - Starts at 0
    - 10s timeout
    - Increments once every 2s, for 10s
    - Including overheads, not expected to reach 5
  - `loop_counter2` = 4
    - Starts at 0
    - 10s timeout
    - Increments once every 1s, for 5s
    - Then aborts

### Fixing it
> [!NOTE]
> Full notes are found in the code.<br>
> See [`cpp_task_modified.cpp`](./cpp_task_modified.cpp)<br>

- Minor cleanup for readability
- Separate `my_running` into 2 variables
  - One for each thread
- Adding basic debug prints makes it clear that:
  - `my_thread1` isn't running correctly
  - `my_thread2` has a race condition
- Changed `StartThread` to take ownership of the lambda
- Running each thread on their own, seems to work ok
  - Further points to race condition / shared data issue
- Atomicity of the loop counters isn't important
- When `StartThread` returns, its scope ends
  - This means its arguments will disappear
  - This means that the thread it spawned now holds dangling references
  - Moved away from references for this

## Issues
### Major
- `my_running` is shared between 2 parallel threads
  - It is atomic to prevent data races
  - However, I assume is not intended to be shared
    - As sharing it leads to separate tasks halting each others' execution
- The `StartThread` function takes the `process` argument by reference
- Using capture-by-reference in async lambdas is dangerous
  - I have experienced this behaviour before first-hand in a test harness
- Timeout functionality in `StartThread` is flawed
  - If `process()` is a blocking call and does not eventually yield, the thread will be stuck forver
- `process()` will continue re-executing until it hits the timeout
  - Should this be a one-shot execution instead?

### Minor
- Classes could be more ergonomic
  - Reduces variable duplication
- Unused include
- Inconsistent formatting / naming
  - Spaces after language keywords
  - Parameter naming
  - Magic numbers
