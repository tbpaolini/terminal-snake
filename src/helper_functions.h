#pragma once

#include "includes.h"

// Remember the game state for clean-up purposes
extern GameState *state_ptr;

// Reset the terminal and its window back to their original states
void cleanup();

// Allocate memory initialized to zero and check if it has been successfully allocated
// Note: program exits on failure.
void* xmalloc(size_t size);

// Move a coordinate by a certain amount in the opposite direction
void offset_coord(GameCoord *coord, SnakeDirection dir, size_t offset);
