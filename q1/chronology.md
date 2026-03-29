# Chronology
## Notes
- This document is presented in chronological format
- This is done so as to document my thought process as accurately as possible
- The actual writeup (condensed) is in [README.md](./README.md)

## Files
- Original question in `cpp_task.cpp`
  - Marked as readonly to prevent accidental modification
- Changed version in `cpp_task_modified.cpp`
- `diff cpp_task.cpp cpp_task_modified.cpp` to see the changes I've made to it

## Chronology
### Question brief
>Question 1:<br>
>Take a look at `cpp_task.cpp`<br>
>What is the code doing, are there any issues, and if so how to fix them?<br>

### Code readthrough - initial observations
- Nothing obviously wrong at first glance

#### Code
- Code is at least C++17 or a later standard
  - Due to use of c++ `chrono_literals`
- Code has access to standard Linux libraries
  - POSIX C Lib, glibc (likely)
  - GNU libstdc++ (direct)

#### Non-issues
- Thread lambda capture-by-reference is fine here

#### Issues
##### Noticed
- Minor readabillity things

##### Clang-tidy
- As part of neovim IDE
- All insignificant - mainly for readability

### Running it
- Assume fairly standard `g++` compilation
  - `g++ --std=c++17 -Wall -Wextra -pedantic <FILE>`
- Output: `C1:0 C2: 6`
  - Would expect `loop_counter1` to be 4
  - Would expect `loop_counter2` to be 4

### Fixing it
- Minor cleanup for readability
- Separate `my_running` into 2 variables - one for each thread
- Add prints for debugging

### Debugging
- Adding basic debug prints makes it clear that:
  - `my_thread1` isn't running correctly
  - `my_thread2` has a race condition
- Changed `StartThread` to take ownership of the lambda

#### GDB
- Usage will interfere with thread timings
- Doesn't show anything particularly useful

#### Continued debugging
- Running each thread on their own, seems to work ok
  - Further points to race condition / shared data issue
- Atomicity of the loop counters isn't important
- When `StartThread` returns, its scope ends
  - This means its arguments will disappear
  - This means that the thread it spawned now holds dangling references




# TODO
- Class variant
- Minimal fixup variant
- use c assert for "unit tests"
- warn that `loop_counter`s should ideally be global (or part of a class ngl)
  - so they're not tied to a specific scope being alive while the threads run
