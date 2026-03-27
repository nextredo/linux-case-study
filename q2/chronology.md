# Chronology
## Notes
- Looks similar to matrix transposition

## Thoughts
- Access to a library like `numpy` would be useful for this
- Different cases we will see
  - Number of matrix rows / cols
    - Even
    - Odd
  - Not square

## Running
- Added extra test case for odd no of elements
- Current algorithm is duplicating elements, skipping others
- Need a temporary storage matrix or a more intelligent in-place approach

## Debugging
- Math appears to check out
- If taking this approach, you need to copy the list by value, not reference
- Also, need to return correctly
