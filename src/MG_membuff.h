#pragma once
/** MG_membuff
 *
 * The idea is to create the most basic kind of memory arena: a buffer with a pointer
 * to the next empty location.
 *
 * I want:
 *
 * - the same effortless allocation I get with a function's stack frame
 * - control over the lifetime of the buffer
 *
 * gameMemBuff = MG_MemBuff()
 */
/** Motiviation
 *
 * This first came up while working on broken-ladder.
 *
 * To draw the player, I started out with the player's vertices defined in the
 * render() function, i.e., these vertices had the lifetime of the render() function.
 * This was fine for animation at first. The initial values of the points were
 * hard-coded in render(). The animation would push the point locations slightly,
 * based on a random value or some other animation effect. Since the initial values
 * were hard-coded, the next frame would "reset" the points to their initial values.
 * This gave the player a fluid appearance without the player's position randomly
 * drifting around the screen.
 *
 * But then I wanted to decouple the animation update rate from the video frame rate.
 * This means the vertices had to persist after exiting render() so that I could draw
 * the vertices for multiple frames with the same value.
 *
 * Stack frame variables do not persist across function calls. I thought of declaring
 * them 'static', but I'm not sure if that is thread-safe.
 *
 * So I simply made the point locations global.
 *
 * This is fine for a single entity. But my game will have many entities. Each will
 * have their own array of points, I'm going to have a lot to keep track of and a lot
 * of tedious setup code to write. A ton of global arrays is hard to organize in code.
 * I have to define ALL of my entitities ahead of time in code: I cannot dynamically
 * create more entities. And how would I store the entities in a saved game file and
 * then read them back later?
 */
/** push_array(), push_struct(), and level lifetime
 *
 * So looking ahead, I want to create my own giant buffer that I can simply push
 * arrays or structs onto. The MemBuff_push_array() or MemBuff_push_struct() should
 * return a pointer to the entity I pushed. I don't need a way to free this memory.
 * The lifetime of all these variables will be similar: perhaps the lifetime is each
 * level of the game (so changing levels ends the life of all variables in this buffer
 * and I get a fresh empty buffer). I guess then this is like a stack frame for
 * levels!
 *
 * So the idea is to reserve a giant array of memory with a lifetime that is the
 * program's lifetime. Then the MemBuff is just a pointer to the next available
 * location in this array and a collection of functions for pushing different types
 * onto the MemBuff, i.e., for allocating memory from this array and updating the
 * MemBuff pointer.
 *
 * A single buffer solves some problems, but not all of them.
 */
/** Two buffers: Index Buffer and Data Buffer
 * To make the contents dynamic, I think I need two buffers: a data buffer and an
 * index buffer for interpreting the contents of the data buffer.
 *
 * For example, the player has a points_index struct that is a 'count' and the base
 * address of the points. The player as a lot of other data, but right now I am only
 * thinking of the data that render() needs. The render() will use the points_index
 * struct to get the base address and the 'count' to know how many points to iterate
 * over.
 *
 * When loading a saved game from file, the base address of the data buffer will be
 * different (the base address changes each time the program runs). To make the
 * addresses in the index buffer robust against this, the index buffer stores offsets
 * from the data buffer base address rather than absolute addresses in the data
 * buffer.
 */
/** MemBuff is a buffer of MEMBUFF_LEN bytes
 * \param array this is the pool of memory for allocating contiguous blocks to the caller
 * \param start this always points at the base address of 'array'
 * \param head this points to the next available memory address in 'array'
 */

#include <stdint.h>
#define MEMBUFF_LEN 8
typedef struct {
    uint8_t array[MEMBUFF_LEN];
    uint8_t* start;
    uint8_t* head;
} MemBuff;

void MemBuff_init(MemBuff* const memBuff) {
    memBuff->start = memBuff->array;
    memBuff->head = memBuff->array;
}

/**
 * Reserve in the `memBuff` (memory buffer) an array of 'count' 'type'.
 * \param memBuff MemBuff (memory buffer) 
 * \param count Array length
 * \param type Data type, e.g., uint8_t or SDL_FPoint.
 *              The macro converts 'type' into a size_t with sizeof(type)
 * \return Return a pointer to the base address of the array. Return NULL if memBuff
 * does not have enough room to store the array.
 */
#define MemBuff_push_array(memBuff, count, type) (type *)MemBuff_push_array_(memBuff, (count)*sizeof(type))
uint8_t* MemBuff_push_array_(MemBuff* const memBuff, size_t size) {
    // Return NULL if there is not enough room left in the buffer to fit the array
    if (memBuff->head + size >= memBuff->start + MEMBUFF_LEN) return NULL;

    // Otherwise, advance head and return the base address of the array
    uint8_t* base_address = memBuff->head;
    memBuff->head += size;
    return base_address;
}

/**
 * Test helper to check a memBuff by printing its pointer addresses and its contents.
 * \param memBuff address of the MemBuff that I want to inspect
 */
void check_membuff(MemBuff* const memBuff) {
    puts("\tInspect memBuff:");
    printf("\t\tmemBuff starts here:          %p\n", (void *)memBuff->start);
    printf("\t\tmemBuff head points here:     %p\n", (void *)memBuff->head);
    printf("\t\tmemBuff is %d bytes long\n", MEMBUFF_LEN);
    printf("\t\tLast byte in memBuff is here: %p\n", (void *)(memBuff->start + MEMBUFF_LEN - 1));
    printf("\t\tContents of memBuff:\n\t\t");
    for (int i=0; i<MEMBUFF_LEN; i++) printf("%d,",memBuff->array[i]);
    puts("");
}

