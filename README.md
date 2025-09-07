# About

Come up with a simple memory buffer for easily allocating memory needed for the
current level of a game.

This is based on the simple idea presented by Casey Muratori in [Episode 43 of
Handmade Hero][^1_1]. Jump to the 43:00 mark for the initial code and the
1:05:00 mark for a diagram explanation.

[^1_1]: https://youtube.com/watch?v=IJYTwhqfKLg&si=aGv8rugixUBomGtA

# MemBuff

The memory buffer, `MemBuff`, is like a function stack frame, but it is for a
game level rather than for a function. A function automatically pops all of its
local variables off the stack, but I will never pop anything, I will simply
"free" the entire array by resetting the pointer to the head of the array.

To summarize, `MemBuff` is just:

- `array`: a giant array of memory with a lifetime that is the program lifetime
- `start`: a pointer to the first address in the giant array (for initialization and for memory free)
- `head`: a pointer to the next available address in the giant array (for the caller to allocate)
- functions `_init`, `_push_array`, and `_push_strcut` for pushing arrays and
  structs onto this giant array

# TDD

I'll use TDD for this. I put the beginnings of a simple test framework in
`src/main.c`. I will add new `TEST_` macros to this as needed.

# Status

## TDD status

* TDD has 7 passing tests.
* All tests follow the format:
    * Setup
    * Exercise
    * Test
* TDD has macros:
    * `TEST_INTeq(a,b,...)` to test if `a == b` for integers
    * `TEST_PTReq(a,b,...)` to test if `a == b` for pointers
    * `TEST_PTRgt(a,b,...)` to test if `a > b`  for pointers
    * The final `...` argument is a `puts("Name or description of test")`

The test output looks like this:

```
Running tests...
 1. PASS: MemBuff_init(&memBuff) sets memBuff.start to point at the first address in memBuff.array
 2. PASS: MemBuff_push_array(&memBuff, 2, uint8_t) advances the MemBuff.head pointer by 2 bytes
 3. PASS: MemBuff_push_array(...) returns NULL if array cannot fit in memBuff
 4. PASS: Given memBuff.array is empty, MemBuff_push_array(&memBuff, 2, uint8_t) returns a pointer to the first address in memBuff.array
 5. PASS: Given memBuff.array is not empty, MemBuff_push_array(&memBuff, 2, uint8_t) returns a pointer to an address after the first address in memBuff.array
 6. PASS: Caller can read and write the first byte after MemBuff_push_array(&memBuff, 2, uint8_t)
 7. PASS: Caller can read and write the second byte after MemBuff_push_array(&memBuff, 2, uint8_t)
        Inspect memBuff:
                memBuff starts here:          0x7ffe449febe0
                memBuff head points here:     0x7ffe449febe2
                memBuff is 8 bytes long
                Last byte in memBuff is here: 0x7ffe449febe7
                Contents of memBuff:
                1,2,0,0,0,0,0,0,
 8. FAIL: A failing test looks like this
        Run TEST in run_tests_for_MemBuff(), src/main.c|313:
        Expect '1 == 2' but '1 == 1'

------------
Test summary
------------
1 (fail) and  7 (pass) out of 8 tests (total)
```

The word `PASS` is green foreground text and `FAIL` has a red background.

## Macro to pass array data type

I used a macro to pass data types instead of having to call `sizeof()` in the
calling code:

The macro lets me do this:

```c
MemBuff_push_array(count, type)
```

Instead of this:

```c
MemBuff_push_array(count, sizeof(*ptr))
```

In the former, the macro and function definitions are:

```c
#define MemBuff_push_array(count, type) (type *)MemBuff_push_array_((count)*sizeof(type))
uint8_t* MemBuff_push_array_(size_t size) {
    uint8_t* address = MemBuff_ptr;
    MemBuff_ptr += size;
    return address;
}
```

In the latter (without the macro), the function definition is:

```c
uint8_t* MemBuff_push_array(int count, int size) {
    uint8_t* address = MemBuff_ptr;
    MemBuff_ptr += count*size;
    return address;
}
```

## Turn MemBuff into a struct

Instead of using a single global memory buffer, I created a `MemBuff` type:

```c
typedef struct {
    uint8_t array[MEMBUFF_LEN];
    uint8_t* start;
    uint8_t* head;
} MemBuff;
```

Define a `MemBuff` to obtain the memory from the OS:

```c
MemBuff memBuff;
```

Above I define `memBuff`.

### MemBuff array does not need to be initialized

Defining a `MemBuff` in this way does allocate the memory for member `array`.
And this allocation happens in whatever scope the declaration happens. As long
as I do `MemBuff membuff;` in `main()`, the lifetime of member `array` is the
lifetime of the program because `main()` does not return until the end of the
program.

I don't initialize the struct because there is nothing to initialize.

For example, I can use array initialization to initialize the first few values
of member `array`, e.g.:

```c
MemBuff memBuff = {.array={1,2,3,4}}
```

This is useful for unit tests. But in the actual application, I never care what
values are in the array for any address from `head` up to the end of the array.

## Initialize MemBuff start and head pointers

And there is no way to assign the pointers `start` and `head` in the structure
initialization. But I do need to assign these pointers, so I created an `_init`
function:

```c
MemBuff_init(&memBuff);
```

All it does is assign `start` and `head` to both point at the first address in
`array`.

```c
void MemBuff_init(MemBuff* const memBuff) {
    memBuff->start = memBuff->array;
    memBuff->head = memBuff->array;
}
```

I define and initialize on the same line:

```c
MemBuff memBuff; MemBuff_init(&memBuff);
```

## Push array of bytes onto MemBuff

*Remember I'm "getting" an array of bytes from the MemBuff pool. "Push" is
referring to the idea of the buffer as a stack. "Push" just means I'm reserving
bytes for the caller to use.*

To push arrays:

```c
#define MemBuff_push_array(memBuff, count, type) (type *)MemBuff_push_array_(memBuff, (count)*sizeof(type))
uint8_t* MemBuff_push_array_(MemBuff* const memBuff, size_t size) {
    // Return NULL if there is not enough room left in the buffer to fit the array
    if (memBuff->head + size >= memBuff->start + MEMBUFF_LEN) return NULL;

    // Otherwise, advance head and return the base address of the array
    uint8_t* base_address = memBuff->head;
    memBuff->head += size;
    return base_address;
}
```

Returning `NULL` indicates there is not enough room to hold the array the caller is asking for.

## Inspect the MemBuff

Here is a helper function (handy during testing) to inspect the memory buffer:

```c
void check_membuff(MemBuff* const memBuff) {
    puts("Inspect memBuff:");
    printf("\tmemBuff starts here:          %p\n", (void *)memBuff->start);
    printf("\tmemBuff head points here:     %p\n", (void *)memBuff->head);
    printf("\tmemBuff is %d bytes long\n", MEMBUFF_LEN);
    printf("\tLast byte in memBuff is here: %p\n", (void *)(memBuff->start + MEMBUFF_LEN - 1));
    printf("\tContents of memBuff:\n\t\t");
    for (int i=0; i<MEMBUFF_LEN; i++) printf("%d,",memBuff->array[i]);
    puts("");
}
```

Check like this: `check_membuff(&memBuff)`. Example output:

```
Inspect memBuff:
        memBuff starts here:          0x7ffffea98710
        memBuff head points here:     0x7ffffea98712
        memBuff is 8 bytes long
        Last byte in memBuff is here: 0x7ffffea98717
        Contents of memBuff:
                1,2,0,0,0,0,0,0,
```

# Next steps

1. Make this work with arrays of larger data types (like floats).
1. Make a similar function for pushing structs.
