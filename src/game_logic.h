#pragma once

#include "includes.h"

// Spawn a food pellet at a random empty space
void spawn_food(GameState *state);

// Get which direction the user has pressed
SnakeDirection parse_input();

// Move the snake by one unit in a given direction
// Return 'true' if the snake has collided with something, 'false' otherwise.
bool move_snake(GameState* state, SnakeDirection dir);

// Bend the snake's body to the direction it is turning to.
// (this function draws the appropriate shape on the point the snake bent)
void snake_turning(GameState* state, SnakeDirection new_dir);
