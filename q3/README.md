# Question 3 - C++ UDP Class Design
> Design and implement a C++ class that offers interface to:<br>
> - Send (immediately) a message over udp to a specific ip & port.<br>
> - Send (with a non-blocking request) a udp message to specific ip & port after `X` seconds' delay.<br>
>   - (where X could be any value between 1s and 255s)<br>
> - Request periodic sending of a udp message to specific ip & port every `X` seconds.<br>
>   - (where X could be any value between 1s and 255s)<br>

## Design Process
### Test Driven Development
- I strongly believe in Test-Driven Development (TDD)
  - Having read and applied [Kent Beck's book on the subject][kb-tdd], I really like how it works for me
- So, the iterative design loop will be:
  - Red - create a test that fails
  - Green - get the test to pass with the bare minimum amount of code
  - Refactor - clean the code up, ensuring it still passes

[kb-tdd]: https://www.amazon.com.au/Test-Driven-Development-Kent-Beck/dp/0321146530
