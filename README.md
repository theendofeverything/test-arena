# About

Come up with a simple memory stack for easily allocating memory needed for the
current level of a game.

This is based on the simple idea presented by Casey Muratori in [Episode 43 of
Handmade Hero][^1_1]. Jump to the 43:00 mark for the initial code and the
1:05:00 mark for a diagram explanation.

[^1_1]: https://youtube.com/watch?v=IJYTwhqfKLg&si=aGv8rugixUBomGtA

# MemStack

The memory stack, `MemStack`, is like a function stack frame, but it is for a
game level rather than for a function. Unlike a function which automatically
pops all of its local variables off the stack, I will never pop anything, I
will simply "free" the entire array by resetting the pointer to the head of the
array.

i.e., `MemStack` is just:

- a giant array of memory with a lifetime that is the program lifetime
- a pointer to the first address in the giant array (for reset/free)
- a pointer to the next available address in the giant array (for allocating)
- functions for pushing arrays and structs onto this giant array

# TDD

I'll use TDD for this. I put the beginnings of a simple test framework in
`src/main.c`. I will add new `TEST_` macros to this as needed.

# Status

Initial TDD framework is in there with two passing tests.

I used a macro to pass data types instead of having to call `sizeof()` in the
calling code:

The macro lets me do this:

```c
MemStack_push_array(count, type)
```

Instead of this:

```c
MemStack_push_array(count, sizeof(*ptr))
```

In the former, the macro and function definitions are:

```c
#define MemStack_push_array(count, type) (type *)MemStack_push_array_((count)*sizeof(type))
uint8_t* MemStack_push_array_(size_t size) {
    uint8_t* address = MemStack_ptr;
    MemStack_ptr += size;
    return address;
}
```

In the latter (without the macro), the function definition is:

```c
uint8_t* MemStack_push_array(int count, int size) {
    uint8_t* address = MemStack_ptr;
    MemStack_ptr += count*size;
    return address;
}
```

# Next steps

1. Throw an assertion error (program terminates) when I run out of memory (this is a temporary placeholder -- long term I probably want to return NULL or -1 to indicate I ran out of memory).
1. Make this work with arrays of larger data types (like floats).
1. Make a similar function for pushing structs.
