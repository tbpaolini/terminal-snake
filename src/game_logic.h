#pragma once

#include "includes.h"

// Spawn a food pellet at a random empty space
void spawn_food(GameState *state);

// Get which direction the user has pressed
SnakeDirection parse_input();
