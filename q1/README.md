# Question 1 - `cpp_task.cpp`
## Document notes
- This is the condensed writeup of the task

## Notes
## Running

```bash
# Compile & run
g++ --std=c++17 cpp_task_modified.cpp && ./a.out

# Compile & debug
# Terminal 1
tty > /tmp/gdbterm
# Terminal 2
g++ --std=c++17 -g cpp_task_modified.cpp && gdb --tty="$(cat /tmp/gdbterm/)" --args ./a.out

# Additional flags include:
# -Wall -Wextra -pedantic
```

- Output
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
