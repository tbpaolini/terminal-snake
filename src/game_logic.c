#include "includes.h"

// Spawn a food pellet at a random empty space
void spawn_food(GameState *state)
{
    // Return if there are no empty spaces for the food to be spawned
    if (state->free_area == 0)
    {
        state->food = (GameCoord){(size_t)-1, (size_t)-1};
        return;
    }
    
    // Randomly pick one of then open spaces
    size_t random_id = xrand() % state->free_area;

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
    //       It's going to run up to 'state->free_area' times.
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

// Get which direction the user has pressed
SnakeDirection parse_input(GameState* state)
{
    SnakeDirection dir = DIR_NONE;
    
    int my_char = 0;
    while (my_char != EOF)
    {
        my_char = getchar_nb();
        
        // Keep reading the input stream until we find the sequence "ESC O"
        // (ESC character followed by the letter O character)
        // Or a sequence that is in the `state->keymap` trie
        
        if (my_char == '\x1b')  // Check if an arrow key was pressed
        {
            my_char = getchar();
            if (my_char != 'O') continue;
            my_char = getchar();

            /* Note: The program locks waiting for input in case the user press the ESC key.
            This was an oversight on the above code, but I am keeping it as a "pause" feature
            because I really liked it :-)
            */

            // The next character after the sequence tells us which direction was pressed
            switch (my_char)
            {
                case 'A':
                    dir = DIR_UP;
                    goto parsing_done;
                
                case 'B':
                    dir = DIR_DOWN;
                    goto parsing_done;
                
                case 'C':
                    dir = DIR_RIGHT;
                    goto parsing_done;
                
                case 'D':
                    dir = DIR_LEFT;
                    goto parsing_done;
                
                default:
                    continue;
            }
        }
        else if (state && state->keymap)    // Check if a mapped character key was pressed
        {
            if (my_char < 0 || my_char > 255) continue;
            KeyMap *node = state->keymap->next[my_char];    // Start from the trie's root
            
            // Check if the byte sequence match any of the sequences in `state->keymap`
            while (node != NULL)
            {
                if (node->dir != DIR_NONE)
                {
                    // The byte sequence was found
                    dir = node->dir;
                    goto parsing_done;
                }
                else
                {
                    // Get the next byte in the sequence
                    my_char = getchar();
                    if (my_char >= 0 && my_char <= 255)
                    {
                        // Move to the trie's next node
                        node = node->next[my_char];
                    }
                    else
                    {
                        node = NULL;
                    }
                }
            }
        }
    }
    
    parsing_done:
    
    // Flush the remaining input
    flush_stdin();
    /* Note: If the user keeps the key pressed, a lot of inputs might get buffered.
       Parsing that input could slow down the program, and it is not really
       necessary to process them once we already got a direction. */
    
    return dir;
}

// Move the snake by one unit in a given direction
// Return 'true' if the snake has collided with something, 'false' otherwise.
bool move_snake(GameState* state, SnakeDirection dir)
{
    if (dir == DIR_NONE) return false;
    
    // Prevent the snake from moving backwards
    correct_direction(state, &dir);
    
    // Bend the snake's body to the new direction
    snake_turning(state, dir);
    
    // Move the snake's head to the new coordinate
    move_coord(&state->position, dir, 1);
    const size_t my_row = state->position.row;
    const size_t my_col = state->position.col;
    
    // Coordinate of the snake's tail
    const GameCoord tail = state->snake[state->tail];
    
    // Check if the snake hit an wall or itself
    // (do not collide with the tail because it is moving away from its current position)
    const bool has_collided = state->arena[my_row-1][my_col-1] && (my_row != tail.row || my_col != tail.col);

    // Check if a food pellet was obtained
    const bool got_food = (state->food.row == my_row) && (state->food.col == my_col);

    // Flag the new head's position as occupied
    state->arena[my_row-1][my_col-1] = true;

    // Push the new head's coordinate into the start of the queue
    state->head = (state->head > 0) ? state->head - 1 : state->total_area - 1; // Wrap around the buffer
    state->snake[state->head] = state->position;

    // Pop the snake's tail from the end of the queue if no food was obtained
    if (!got_food)
    {
        // Flag the old tail's position as empty
        const GameCoord pos = state->snake[state->tail];
        state->arena[pos.row-1][pos.col-1] = false;

        // Delete the tail's end from the screen
        printf(MOVE_CURSOR(%zu,%zu) " ", pos.row, pos.col);

        // Remove the old coordinate from the queue
        state->tail = (state->tail > 0) ? state->tail - 1 : state->total_area -1;  // Wrap around the buffer
    }
    else
    {
        // Generate a new food pellet after one was eaten
        spawn_food(state);
        
        // There is one less empty space since the snake has grown
        state->free_area -= 1;

        // The snake grows when getting the food pellet
        state->size += 1;

        // Update the size counter that is shown on the screen
        print_snake_size(state);
    }

    // Draw the head at the new position
    // (it is being drawn last in order to avoid being cleared in case the head ends up in the tail's old position)
    draw_snake_head(state, has_collided);

    fflush(stdout);

    #ifdef _WIN32
    // On Windows, this prevents events other than key presses from cluttering the input buffer.
    // This is needed because we are checking the event count in order to determine if there's a key to be parsed.
    // We do not really need to handle those other events on this program.
    FlushConsoleInputBuffer(state->input_handle);
    #endif

    return has_collided;
}

// Prevent the snake from moving backwards
// This function flips the new direction in case it's going to the opposite direction of the snake.
extern inline void correct_direction(GameState* state, SnakeDirection* new_dir)
{
    const SnakeDirection old_dir = state->direction;
    switch (old_dir)
    {
        case DIR_UP:
            if (*new_dir == DIR_DOWN) *new_dir = DIR_UP;
            break;
        
        case DIR_DOWN:
            if (*new_dir == DIR_UP) *new_dir = DIR_DOWN;
            break;
        
        case DIR_RIGHT:
            if (*new_dir == DIR_LEFT) *new_dir = DIR_RIGHT;
            break;
        
        case DIR_LEFT:
            if (*new_dir == DIR_RIGHT) *new_dir = DIR_LEFT;
            break;
        
        default:
            break;
    }
}

// Bend the snake's body to the direction it is turning to.
// (this function draws the appropriate shape on the point the snake bent)
void snake_turning(GameState* state, SnakeDirection new_dir)
{
    // Move the terminal's cursor to where the snake's head is
    printf(MOVE_CURSOR(%zu,%zu), state->position.row, state->position.col);
    const SnakeDirection old_dir = state->direction;

    if (old_dir == new_dir) // Snake did not change its direction
    {
        if (new_dir == DIR_UP || new_dir == DIR_DOWN)
        {
            printf(TEXT_GREEN SNAKE_VERTICAL);
        }
        else if (new_dir == DIR_LEFT || new_dir == DIR_RIGHT)
        {
            printf(TEXT_GREEN SNAKE_HORIZONTAL);
        }
        else printf(TEXT_GREEN "?");
    }
    else // Snake turned to another direction
    {
        switch (old_dir)
        {
            case DIR_UP:
                if (new_dir == DIR_LEFT)
                {
                    printf(TEXT_GREEN SNAKE_TOP_RIGHT);
                }
                else if (new_dir ==  DIR_RIGHT)
                {
                    printf(TEXT_GREEN SNAKE_TOP_LEFT);
                }
                else printf(TEXT_GREEN "?");
                break;
            
            case DIR_DOWN:
                if (new_dir == DIR_LEFT)
                {
                    printf(TEXT_GREEN SNAKE_BOTTOM_RIGHT);
                }
                else if (new_dir == DIR_RIGHT)
                {
                    printf(TEXT_GREEN SNAKE_BOTTOM_LEFT);
                }
                else printf(TEXT_GREEN "?");
                break;
            
            case DIR_LEFT:
                if (new_dir == DIR_UP)
                {
                    printf(TEXT_GREEN SNAKE_BOTTOM_LEFT);
                }
                else if (new_dir == DIR_DOWN)
                {
                    printf(TEXT_GREEN SNAKE_TOP_LEFT);
                }
                else printf(TEXT_GREEN "?");
                break;
            
            case DIR_RIGHT:
                if (new_dir == DIR_UP)
                {
                    printf(TEXT_GREEN SNAKE_BOTTOM_RIGHT);
                }
                else if (new_dir == DIR_DOWN)
                {
                    printf(TEXT_GREEN SNAKE_TOP_RIGHT);
                }
                else printf(TEXT_GREEN "?");
                break;
            
            default:
                printf(TEXT_GREEN "?");
                break;
        }
    }

    // Store the new direction
    state->direction = new_dir;
}

// Draw the snake's head according to its direction and position
void draw_snake_head(GameState* state, bool has_collided)
{
    printf(MOVE_CURSOR(%zu,%zu), state->position.row, state->position.col);

    // The head's color is red in case of collision, green otherwise
    if (has_collided)
    {
        printf(TEXT_RED);
    }
    else
    {
        printf(TEXT_GREEN);
    }
    
    switch (state->direction)
    {
        case DIR_RIGHT:
            printf(SNAKE_HEAD_RIGHT);
            break;
        
        case DIR_LEFT:
            printf(SNAKE_HEAD_LEFT);
            break;
        
        case DIR_DOWN:
            printf(SNAKE_HEAD_DOWN);
            break;
        
        case DIR_UP:
            printf(SNAKE_HEAD_UP);
            break;
        
        default:
            printf("?");
            break;
    }
}

// Display at the bottom of the screen the current snake's size
void print_snake_size(GameState *state)
{
    printf(
        MOVE_CURSOR(%zu,%d) TEXT_GRAY "Size: " TEXT_WHITE "%zu",
        state->screen_size.row, SCREEN_MARGIN + 1, state->size
    );
}

// Print the game over message
void game_over(GameState *state)
{
    print_snake_size(state); // Has the side effect of moving the cursor to after the size value
            
    if (state->free_area == 0)
    {
        printf(TEXT_YELLOW "  CONGRATULATIONS!");
    }
    else printf(TEXT_RED "  GAME OVER!");
    
    printf(TEXT_CYAN "  Press any key to exit...\a"); // '\a' makes a notification sound on the terminal
    fflush(stdout);

    // Wait a little without taking input so the user does not accidentally exit at the end
    wait_usec(750000);  // 0.75 seconds
    flush_stdin();
    #ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 6031)  // Disable the "unused return value" warning on the MSVC compiler's analyzer
    #endif
    getchar();  // Wait for the user to press any key to exit
    #ifdef _MSC_VER
    #pragma warning(pop)
    #endif
    flush_stdin();  // So any remaining input isn't echoed on the terminal
}
