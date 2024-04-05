#pragma once

#include "includes.h"

// Spawn a food pellet at a random empty space
void spawn_food(GameState *state);

// Get which direction the user has pressed
SnakeDirection parse_input(GameState* state);

// Move the snake by one unit in a given direction
// Return 'true' if the snake has collided with something, 'false' otherwise.
bool move_snake(GameState* state, SnakeDirection dir);

// Prevent the snake from moving backwards
// This function flips the new direction in case it's going to the opposite direction of the snake.
inline void correct_direction(GameState* state, SnakeDirection* new_dir);

// Bend the snake's body to the direction it is turning to.
// (this function draws the appropriate shape on the point the snake bent)
void snake_turning(GameState* state, SnakeDirection new_dir);

// Draw the snake's head according to its direction and position
// (draw in red in case of collision)
void draw_snake_head(GameState* state, bool has_collided);

// Display at the bottom of the screen the current snake's size
void print_snake_size(GameState *state);

// Print the game over message
void game_over(GameState *state);
