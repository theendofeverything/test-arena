#include <stdio.h>
#include <stdbool.h>
#include "MG_membuff.h"

//////////////////////
// UNIT TEST FRAMEWORK
//////////////////////
int MGUT_total = 0;
int MGUT_pass = 0;
int MGUT_fail = 0;
bool MGUT_show_failing_tests = true;
bool MGUT_show_passing_tests = true;

////////////////////////////////////////
// PRIVATE UNIT TEST MACROS FOR PRINTING
////////////////////////////////////////
#define _LOG_TESTNAME(CODE_TO_PRINT_RESULT, CODE_TO_PRINT_TESTNAME) do\
    {\
        printf("%2d. ", MGUT_total + 1);\
        CODE_TO_PRINT_RESULT\
        CODE_TO_PRINT_TESTNAME\
    } while(0)

#define _PRINT_RUN_MSG(TYPE) do\
    {\
    printf("\tRun %s in %s(), %s|%d:\n", #TYPE, __func__, __FILE__, __LINE__);\
    } while(0)

#define _PASSING_TEST(CODE_TO_PRINT_TESTNAME) do\
    {\
        if (MGUT_show_passing_tests)\
        {\
            _LOG_TESTNAME(printf("\033[32mPASS\033[00m: ");, CODE_TO_PRINT_TESTNAME);\
            MGUT_pass++;\
        }\
    } while(0);

////////////////
// UNIT TEST API
////////////////
// Test ints a,b are equal
#define TEST_INTeq(a,b,CODE_TO_PRINT_TESTNAME) if (a!=b)\
    {\
        if (MGUT_show_failing_tests)\
        {\
            _LOG_TESTNAME(printf("\033[41mFAIL\033[00m: ");, CODE_TO_PRINT_TESTNAME);\
            _PRINT_RUN_MSG(TEST);\
            printf("\tExpect '%s == %d' but '%s == %d'\n\n",\
                            #a,    b,      #a,    a);\
        }\
        MGUT_fail++;\
    }\
    else _PASSING_TEST(CODE_TO_PRINT_TESTNAME)\
    MGUT_total++;

// Test pointers a,b are equal
#define TEST_PTReq(a,b,CODE_TO_PRINT_TESTNAME) if (a!=b)\
    {\
        if (MGUT_show_failing_tests)\
        {\
            _LOG_TESTNAME(printf("\033[41mFAIL\033[00m: ");, CODE_TO_PRINT_TESTNAME);\
            _PRINT_RUN_MSG(TEST);\
            printf("\tExpect '%s == %p' but '%s == %p'\n\n",\
                            #a,(void *)b,  #a,(void *)a);\
        }\
        MGUT_fail++;\
    }\
    else _PASSING_TEST(CODE_TO_PRINT_TESTNAME)\
    MGUT_total++;

// Test pointer a > pointer b
#define TEST_PTRgt(a,b,CODE_TO_PRINT_TESTNAME) if (a<=b)\
    {\
        if (MGUT_show_failing_tests)\
        {\
            _LOG_TESTNAME(printf("\033[41mFAIL\033[00m: ");, CODE_TO_PRINT_TESTNAME);\
            _PRINT_RUN_MSG(TEST);\
            printf("\tExpect '%s > %p' but '%s == %p'\n\n",\
                            #a,(void *)b,  #a,(void *)a);\
        }\
        MGUT_fail++;\
    }\
    else _PASSING_TEST(CODE_TO_PRINT_TESTNAME)\
    MGUT_total++;

/////////////
// UNIT TESTS
/////////////
void run_tests_for_MemBuff() {
    { // MemBuff_init() sets memBuff.start to first address in memBuff.array
        // Setup and Exercise
        MemBuff memBuff = {.array={1,2,3,4}}; MemBuff_init(&memBuff);

        // Test
        uint8_t *expected = &memBuff.array[0];
        TEST_PTReq(memBuff.start, expected,
                puts("MemBuff_init(&memBuff) sets memBuff.start to "
                     "point at the first address in memBuff.array");
                );
    }
    { // MemBuff_push_array() advances the MemBuff.head pointer
        // Setup
        MemBuff memBuff; MemBuff_init(&memBuff);

        // Exercise
        MemBuff_push_array(&memBuff, 2, uint8_t);

        // Test
        uint8_t* expected = &memBuff.array[2];
        TEST_PTReq(memBuff.head,expected,
                puts("MemBuff_push_array(&memBuff, 2, uint8_t) "
                     "advances the MemBuff.head pointer by 2 bytes");
                );
    }
    { // MemBuff_push_array() returns NULL if array cannot fit in memBuff
        // Setup
        MemBuff memBuff; MemBuff_init(&memBuff);

        // Exercise
        uint8_t* byte_array = MemBuff_push_array(&memBuff, MEMBUFF_LEN+1, uint8_t);

        // Test
        TEST_PTReq(byte_array,NULL,
                puts("MemBuff_push_array(...) returns NULL if "
                     "array cannot fit in memBuff");
                );
    }
    { // If memBuff is empty, MemBuff_push_array() returns a pointer to first address
        // Setup
        MemBuff memBuff; MemBuff_init(&memBuff);

        // Exercise
        uint8_t* byte_array = MemBuff_push_array(&memBuff, 2, uint8_t);

        // Test
        uint8_t* expected = &memBuff.array[0];
        TEST_PTReq(byte_array, expected,
                puts("Given memBuff.array is empty, "
                     "MemBuff_push_array(&memBuff, 2, uint8_t) "
                     "returns a pointer to the first address in memBuff.array");
                );
    }
    { // If memBuff is not empty, MemBuff_push_array() returns a pointer to a later address
        // Setup
        MemBuff memBuff; MemBuff_init(&memBuff);
        MemBuff_push_array(&memBuff, 2, uint8_t);

        // Exercise
        uint8_t* byte_array = MemBuff_push_array(&memBuff, 2, uint8_t);

        // Test
        uint8_t* expected = &memBuff.array[0];
        TEST_PTRgt(byte_array, expected,
                puts("Given memBuff.array is not empty, "
                     "MemBuff_push_array(&memBuff, 2, uint8_t) "
                     "returns a pointer to the address two bytes after the "
                     "first address in memBuff.array");
                );
    }
    { // Caller can r/w first N bytes after pushing an N-byte array
        // Setup
        MemBuff memBuff = {.array={0,0}}; MemBuff_init(&memBuff);

        // Exercise
        uint8_t* byte_array = MemBuff_push_array(&memBuff, 2, uint8_t);

        // Test
        uint8_t a = 1, b = 2;
        byte_array[0] = a; byte_array[1] = b;
        TEST_INTeq(byte_array[0], a,
                puts("Caller can read and write the first byte after "
                     "MemBuff_push_array(&memBuff, 2, uint8_t)");
                );
        TEST_INTeq(byte_array[1], b,
                puts("Caller can read and write the second byte after "
                     "MemBuff_push_array(&memBuff, 2, uint8_t)");
                );
        if (0) check_membuff(&memBuff);
    }
}

//////////////
// TEST RUNNER
//////////////
int main(void) {
    puts("Running tests...");
    run_tests_for_MemBuff();
    puts("------------");
    puts("Test summary");
    puts("------------");
    printf("%d (fail) and  %d (pass) out of %d tests (total)\n",
            MGUT_fail,MGUT_pass,MGUT_total);
}
