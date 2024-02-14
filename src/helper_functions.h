#pragma once

#include "includes.h"

// Remember the game state for clean-up purposes
extern GameState *state_ptr;

// Reset the terminal and its window back to their original states
void cleanup();

// Allocate memory initialized to zero and check if it has been successfully allocated
// Note: program exits on failure.
void* xmalloc(size_t size);

// Allocate memory for a two dimensional array and initialize its elements to zero
// Layout: array[height][width]
// Note: the returned pointer can be passed to free() to get the entire array freed at once.
void** alloc_2Darray(size_t width, size_t height, size_t element_size);

// Move in-place a coordinate by a certain offset in the given direction
void move_coord(GameCoord *coord, SnakeDirection dir, size_t offset);

// Generate a pseudo-random unsigned integer from 0 to UINT_MAX
// Note: this function uses the entropy source from the operating system, so seeding is not needed.
unsigned int xrand();

// Reset the terminal size to the original values
// Note: this function is meant to be called when the terminal window is resized on Linux (SIGWINCH signal)
void restore_term(int signal);
