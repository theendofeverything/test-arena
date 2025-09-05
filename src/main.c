/** MG_mem_stack
 *
 * The idea is to create the most basic kind of memory arena: a stack.
 *
 * I want the same kind of stack I get with a function's stack frame, except I want
 * this stack to be independent of the lifetime of any function's stack frame.
 *
 * This first came up when, while working on broken-ladder, I wanted to control the
 * update physics timing: instead of updating point locations on every frame (every
 * iteration of the game loop), I only want to update them at some slower rate. I was
 * using the stack frame of my render() function to allocate the memory for the point
 * locations. So this was allocated (and automatically deallocated) on every frame. I
 * wanted the point locations to return to their initial value, then as part of the
 * animation, I would push the point locations slightly, based on a random value or
 * some other animation effect.
 *
 * For the value to persist across frames, I cannot use the render() stackframe
 * because all of that memory is deallocated when the function exits.
 *
 * So I simply made the point locations global.
 *
 * This is fine, but now when I want to add more entities with their own array of
 * points, I'm going to have a lot to keep track of. I can simply create a ton of
 * global arrays, but this is hard to organize in source code and it means I need to
 * define all of my entitities ahead of time: I cannot dynamically create more
 * entities.
 *
 * So looking ahead, I want to create my own giant stack that I can simply push arrays
 * or structs onto. The MemStack_push_array() or MemStack_push_struct() should return
 * a pointer to the entity I pushed. I don't need a way to free this memory. The
 * lifetime of all these variables will be similar: perhaps the lifetime is each level
 * of the game (so changing levels ends the life of all variables on this stack and I
 * get a fresh empty stack). I guess then this is like a stackframe for levels!
 *
 * So the idea is to reserve a giant array of memory with a lifetime that lasts for
 * the entire program. Then the MemStack is just a pointer to the next available
 * location in this array and a collection of functions for pushing different types
 * onto the MemStack, i.e., for allocating memory from this array and updating the
 * MemStack pointer.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

//////////////////////
// UNIT TEST FRAMEWORK
//////////////////////
int MGUT_total = 0;
int MGUT_pass = 0;
int MGUT_fail = 0;
bool MGUT_show_tests = true;

#define HEADING(TYPE) do\
    {\
    printf("\nRun %s in %s(), %s|%d:", #TYPE, __func__, __FILE__, __LINE__);\
    } while(0)

// Test ints a,b are equal
#define TEST_INTeq(a,b) if (a!=b)\
    {\
        if (MGUT_show_tests)\
        {\
            HEADING(TEST);\
            printf(" \033[41mFAIL\033[00m\n\t");\
            printf("Expect '%s == %d' but '%s == %d'",\
                            #a,    b,      #a,    a);\
        }\
        MGUT_fail++;\
    }\
    else MGUT_pass++;\
    MGUT_total++;

// Test pointers a,b are equal
#define TEST_PTReq(a,b) if (a!=b)\
    {\
        if (MGUT_show_tests)\
        {\
            HEADING(TEST);\
            printf(" \033[41mFAIL\033[00m\n\t");\
            printf("Expect '%s == %p' but '%s == %p'",\
                            #a,(void *)b,  #a,(void *)a);\
        }\
        MGUT_fail++;\
    }\
    else MGUT_pass++;\
    MGUT_total++;

///////////////////////////
// MemStack CODE UNDER TEST
///////////////////////////
/** MemStack is an array of MEMSTACK_LEN bytes
 * TODO: put the variables below inside of a struct named MemStack (this is the doc
 * string for that struct!)
 * \param
 */
#define MEMSTACK_LEN 256
uint8_t MemStack_arr[MEMSTACK_LEN];
uint8_t* MemStack_ptr = MemStack_arr;
/**
 * Return a pointer to 'count' * 'size' number of bytes
 * \param count Array length
 * \param size Size of one element in the array
 */
uint8_t* MemStack_push_array(int count, int size) {
    uint8_t* address = MemStack_ptr;
    MemStack_ptr += count*size;
    return address;
}

/////////////
// UNIT TESTS
/////////////
void run_tests_for_MG_stack() {
    { // MemStack_push_array() returns a pointer
        puts("Given MemStack_arr is empty, "
                "MemStack_push_array(2, sizeof (*byte_array) "
                "returns a pointer to the first address in MemStack_arr");
        uint8_t* expected = &MemStack_arr[0];
        uint8_t* byte_array;
        byte_array = MemStack_push_array(2, sizeof (*byte_array));
        TEST_PTReq(byte_array, expected);
    }
    { // MemStack_push_array() advances the MemStack pointer
        puts("MemStack_push_array(2, sizeof (*byte_array) "
                "advances the MemStack pointer by 2 bytes");
        uint8_t* expected = MemStack_ptr+2;
        uint8_t* byte_array;
        MemStack_push_array(2, sizeof (*byte_array));
        TEST_PTReq(MemStack_ptr,expected);
    }
}

//////////////
// TEST RUNNER
//////////////
int main(void) {
    puts("Running tests...");
    run_tests_for_MG_stack();
    puts("------------");
    puts("Test summary");
    puts("------------");
    printf("%d (fail) and  %d (pass) out of %d tests (total)\n",
            MGUT_fail,MGUT_pass,MGUT_total);
}
