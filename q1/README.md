# Question 1 - `cpp_task.cpp`
## Notes
- This document is presented in chronological format
- This is done so as to document my thought process as accurately as possible

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

