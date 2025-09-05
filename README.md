# About

Come up with a simple memory stack for easily allocating memory needed for the
current level of a game.

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

Initial TDD framework is in there with two passing tests. Next steps:

1. Use a macro so that I can simply do:

```c
MemStack_push_array(count, ptr)
```

Instead of:

```c
MemStack_push_array(count, sizeof(*ptr))
```

After that, do I even need to return the pointer? Why not pass `&ptr` instead
of `ptr` and then I can do `*ptr = MemStack_ptr`.

2. Throw an assertion error (program terminates) when I run out of memory (this is a temporary placeholder -- long term I probably want to return NULL to indicate I ran out of memory).
3. Make this work with arrays of larger data types (like floats).
4. Make a similar function for pushing structs.
