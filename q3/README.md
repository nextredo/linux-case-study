# Question 3 - C++ UDP Class Design
> Design and implement a C++ class that offers interface to:<br>
> - Send (immediately) a message over udp to a specific ip & port.<br>
> - Send (with a non-blocking request) a udp message to specific ip & port after `X` seconds' delay.<br>
>   - (where X could be any value between 1s and 255s)<br>
> - Request periodic sending of a udp message to specific ip & port every `X` seconds.<br>
>   - (where X could be any value between 1s and 255s)<br>

## Notes
- Some `WARN`, `NOTE` or `TODO` comments may be left in the codebase
- These show possible future improvements that could be made should project scope or time permit it.

## Design Process
### Test Driven Development
- I strongly believe in Test-Driven Development (TDD)
  - Having read and applied [Kent Beck's book on the subject][kb-tdd], I really like how it works for me
- So, the iterative design loop has been:
  - Red - create a test that fails
  - Green - get the test to pass with the bare minimum amount of code
  - Refactor - clean the code up, ensuring it still passes
- This approach works well once you get the ball rolling
  - Once unit tests are in place, you can objectively verify if written code matches the API and expectations
  - This achieves separation of "code that works" and "code that's clean", allowing you to approach those two problems independently

## Building
- Done through CMake

### Basic
```bash
# Make build dir, cd into it
mkdir build && cd build

# Create the build system
cmake ..

# Run registered unit tests
ctest
```

### Advanced
```bash
# Generate the build system and compile with debug symbols:
cmake -DCMAKE_BUILD_TYPE=Debug .. && make

# Once built, run a specific test binary and
# add the `-s` switch to show passing tests. e.g.
./build/test/udp_class_tests -s

# Make documentation
make doc
```

[kb-tdd]: https://www.amazon.com.au/Test-Driven-Development-Kent-Beck/dp/0321146530
