#include "includes.h"

// Spawn a food pellet at a random empty space
void spawn_food(GameState *state)
{
    // Randomly pick one of then open spaces
    size_t random_id = xrand() % state->open_count;

    // Current position
    size_t row = state->position_min.row - 1;
    size_t col = state->position_min.col - 1;
    
    // Minimum and maximum positions
    const size_t col_start = col;
    const size_t row_max = state->position_max.row - 1;
    const size_t col_max = state->position_max.col - 1;

    // Collision grid ('false' means an empty space)
    bool** arena = state->arena;
    
    // Index counter for the empty space where the food will end up
    size_t food_id = 0;
    
    // Count the empty spaces on the grid in order to find the randomly chosen space
    // Note: this loop is going to run at least once, even if 'random_id' is zero.
    //       It's going to run up to 'state->open_count' times.
    while (true)
    {
        // Check if we are still inside the snake's area
        if (row > row_max)
        {
            printf_error_exit(ERR_ARRAY_OVERFLOW, "Tried to access an out-of-bounds coordinate.");
        }
        
        if (!arena[row][col])
        {
            // Increment the count if the current space is empty
            food_id++;
            if (food_id > random_id) break;
        }
        
        // Moving to the next space
        if (col < col_max)
        {
            // Go to the next column
            col++;
        }
        else
        {
            // Go to the next row if at the end of a column
            col = col_start;
            row++;
        }
    }
    
    // Put the food pellet at the chosen position
    // Note: the arena[] array is 0-indexed, while the terminal's coordinate is 1-indexed.
    state->food = (GameCoord){
        .row = row + 1,
        .col = col + 1,
    };
    
    printf(
        MOVE_CURSOR(%zu,%zu) TEXT_YELLOW FOOD_PELLET,
        state->food.row, state->food.col
    );
}
