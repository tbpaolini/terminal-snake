#include "includes.h"

// Set-up the game state and draw the initial screen
// 'speed' is a value from 1 to 12 for setting the initial snake's speed.
// 5 is the default speed. 1 is half of the default and 9 is double of the default.
// Other values are a linear interpolation between those points.
GameState* game_init(unsigned int speed)
{
    // Reset the terminal to its default properties when the program exits
    atexit(&cleanup);         // Run our clean-up routine on exit
    signal(SIGINT, &exit);    // Closing with Ctrl+C
    signal(SIGTERM, &exit);   // Closing through task manager
    #if defined(_WIN32) && defined(SIGBREAK)
    signal(SIGBREAK, &exit);  // Closing with Ctrl+Break on Windows
    #endif // _WIN_32 && SIGBREAK
    signal(SIGSEGV, &exit_segfault);    // Closing due to a segmentation fault (this should never happen)

    // Allocate and initialize the game state
    GameState *state = xmalloc(sizeof(GameState));
    state_ptr = state;

    #ifdef _WIN32

    // Make the terminal to accept Unicode characters encoded in UTF-8
    state->output_cp_old = GetConsoleOutputCP();
    WINDOWS_ERROR_CHECK(SetConsoleOutputCP(CP_UTF8));

    // Get the Windows' handles for the standard input and output streams
    state->output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    state->input_handle = GetStdHandle(STD_INPUT_HANDLE);

    // Get the settings of the output stream
    WINDOWS_ERROR_CHECK(GetConsoleMode(state->output_handle, &state->output_mode));
    state->output_mode_old = state->output_mode;

    // Get the settings of the input stream
    WINDOWS_ERROR_CHECK(GetConsoleMode(state->input_handle, &state->input_mode));
    state->input_mode_old = state->input_mode;

    // Get the style of the window where the terminal is
    state->window = GetConsoleWindow();
    WINDOWS_ERROR_CHECK( state->window_mode = GetWindowLong(state->window, GWL_STYLE) );
    state->window_mode_old = state->window_mode;

    // Enable escape sequences on the terminal,
    // and do not move to the next line when writing a character to the end of a line
    state->output_mode |= (ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);
    WINDOWS_ERROR_CHECK(SetConsoleMode(state->output_handle, state->output_mode));
    windows_vt_seq = true;

    // Enable input through the arrows on the terminal.
    // Also make so new characters overwrite existing ones,
    // the input is sent immediately after a key is pressed,
    // and ignore mouse input.
    state->input_mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
    state->input_mode &= (~ENABLE_INSERT_MODE & ~ENABLE_LINE_INPUT & ~ENABLE_ECHO_INPUT & ~ENABLE_MOUSE_INPUT &~ENABLE_WINDOW_INPUT);
    WINDOWS_ERROR_CHECK(SetConsoleMode(state->input_handle, state->input_mode));
    
    // For the terminal's window, disable: maximizing button, resizing, horizontal and vertical scrollbars
    state->window_mode &= (~WS_MAXIMIZEBOX & ~WS_SIZEBOX & ~WS_HSCROLL & ~WS_VSCROLL);
    WINDOWS_ERROR_CHECK(SetWindowLong(state->window, GWL_STYLE, state->window_mode));

    // Get the amount of rows and columns that are visible on the terminal window
    CONSOLE_SCREEN_BUFFER_INFO buffer_info = {0};
    WINDOWS_ERROR_CHECK(GetConsoleScreenBufferInfo(state->output_handle, &buffer_info));
    
    state->screen_size = (GameCoord){
        buffer_info.srWindow.Bottom - buffer_info.srWindow.Top + 1,
        buffer_info.srWindow.Right - buffer_info.srWindow.Left + 1,
    };

    #else // Linux

    // Disable input echoing and make the input to be available immediately
    // (that is, no need for a line to be entered in order for us to read the user's input)
    struct termios term_flags = {0};
    LINUX_ERROR_CHECK(tcgetattr(STDIN_FILENO, &term_flags));
    state->term_flags_old = term_flags;
    term_flags.c_lflag &= (~ECHO & ~ICANON);
    LINUX_ERROR_CHECK(tcsetattr(STDIN_FILENO, TCSANOW, &term_flags));
    linux_term_flags_set = true;

    // Get the amount of rows and columns that are visible on the terminal window
    struct winsize term_size = {0};
    LINUX_ERROR_CHECK(ioctl(STDOUT_FILENO, TIOCGWINSZ, &term_size));
    state->screen_size = (GameCoord){
        term_size.ws_row,
        term_size.ws_col,
    };

    // Change the terminal back to the original size when the window is resized
    signal(SIGWINCH, &restore_term);

    #endif // _WIN_32

    // Map the directions to the "WASD" equivalent keys of the current keyboard layout
    state->keymap = map_scancodes(SCANCODE_UP, SCANCODE_LEFT, SCANCODE_DOWN, SCANCODE_RIGHT);
    if (!state->keymap)
    {
        printf_error_exit(ERR_KEYMAP_FAIL, "Failed to map the keyboard keys.");
    }

    // Distance from the borders of the window in which the snake may not spawn
    const size_t safety_distance = SCREEN_MARGIN + SNAKE_START_SIZE + 1;
    const size_t size_cutoff = 2 * safety_distance;

    // Check if the terminal is big enough for the game
    if ( (state->screen_size.row <= size_cutoff) || (state->screen_size.col <= size_cutoff) )
    {
        printf_error_exit(
            ERR_TINY_TERMINAL,
            "Terminal's size is too small for the game, "
            "it should be at least %zu by %zu characters.",
            size_cutoff+1, size_cutoff+1
        );
    }

    // Top left coordinates of the board
    const GameCoord board_start = {
        SCREEN_MARGIN + 1,
        SCREEN_MARGIN + 1,
    };

    // Bottom right coordinates of the board
    const GameCoord board_end = {
        state->screen_size.row - SCREEN_MARGIN,
        state->screen_size.col - SCREEN_MARGIN,
    };

    // Switch to the alternate terminal screen.
    // Then make its background black, its text light grey, enable keyboard input, and hide the cursor.
    printf(ALT_SCREEN BG_BLACK TEXT_GRAY CLEAR_SCREEN KP_APP_MODE CK_APP_MODE HIDE_CURSOR);

    // Change the window's title to "Snake Game"
    printf(SET_WINDOW_TITLE("Snake Game"));

    // Set the output stream to fully buffered so it is only drawn when we flush it
    setvbuf(stdout, NULL, _IOFBF, state->screen_size.col * state->screen_size.row * 4);

    // Set the input stream to unbuffered so keyboard input can be parsed faster and be cleared more easily
    // Notes: We are going to parse input once per frame. Once we successfully parse a key, we clear the remaining data on stdin.
    //        If stdin was buffered some data could still be on the buffer, which wouldn't be cleared after flushing stdin.
    setvbuf(stdin, NULL, _IONBF, 0);

    // 2D array for the collision grid
    // ('true' means a position where the snake collides with an wall or itself)
    state->arena = (bool**)alloc_2Darray(
        state->screen_size.col,
        state->screen_size.row,
        sizeof(typeof(**state->arena))
    );

    /* Drawing a rectangle along the terminal's borders */
    
    // Draw the top border
    printf(MOVE_CURSOR(%zu, %zu) BOX_TOP_LEFT, board_start.row, board_start.col);
    state->arena[board_start.row - 1][board_start.col - 1] = true;
    
    const size_t top_count = state->screen_size.col - 2 * (SCREEN_MARGIN + 1);
    for (size_t i = 0; i < top_count; i++)
    {
        printf(BOX_HORIZONTAL);
        state->arena[board_start.row - 1][board_start.col + i] = true;
    }

    printf(BOX_TOP_RIGHT);
    state->arena[board_start.row - 1][board_start.col + top_count] = true;
    
    // Draw the laterals
    for (size_t i = board_start.row + SCREEN_MARGIN; i <= board_end.row; i++)
    {
        printf(MOVE_CURSOR(%zu, %zu) BOX_VERTICAL MOVE_CURSOR(%zu, %zu) BOX_VERTICAL, i, board_start.col, i, board_end.col);
        state->arena[i - 1][board_start.col - 1] = true;
        state->arena[i - 1][board_end.col - 1] = true;
    }

    // Draw the bottom border
    printf(MOVE_CURSOR(%zu, %zu) BOX_BOTTOM_LEFT, board_end.row, board_start.col);
    state->arena[board_end.row - 1][board_start.col - 1] = true;

    const size_t bottom_count = state->screen_size.col - 2 * (SCREEN_MARGIN + 1);
    for (size_t i = 0; i < bottom_count; i++)
    {
        printf(BOX_HORIZONTAL);
        state->arena[board_end.row - 1][board_start.col + i] = true;
    }
    
    printf(BOX_BOTTOM_RIGHT);
    state->arena[board_end.row - 1][board_start.col + bottom_count] = true;

    /* Snake spawning */

    // Region in which the snake can spawn
    state->position_min = (GameCoord){
        .row = SCREEN_MARGIN + 2,
        .col = SCREEN_MARGIN + 2,
    };

    state->position_max = (GameCoord){
        .row = state->screen_size.row - (SCREEN_MARGIN + 1),
        .col = state->screen_size.col - (SCREEN_MARGIN + 1),
    };

    // Count the amount of spaces on the snake's area
    const GameCoord box_size = {
        .row = state->position_max.row - state->position_min.col + 1,
        .col = state->position_max.col - state->position_min.col + 1,
    };
    state->total_area = box_size.row * box_size.col;
    state->free_area = state->total_area - SNAKE_START_SIZE;

    // Double-ended queue for storing the coordinates of where each snake part is
    state->snake = xmalloc(sizeof(typeof(*state->snake)) * state->total_area);
    size_t sid = 0; // Current index on 'state->snake[]'

    // Region in which the snake may spawn
    // (there must be a minimum of 1 empty space between the snake's tail and the wall)
    const GameCoord region_min = (GameCoord){
        .row = safety_distance,
        .col = 1 + safety_distance,
    };
    const GameCoord region_max = (GameCoord){
        .row = 1 + state->screen_size.row - safety_distance,
        .col = state->screen_size.col - safety_distance,
    };
    const GameCoord region_size = {
        .row = region_max.row - region_min.row + 1,
        .col = region_max.col - region_min.col + 1,
    };

    // Randomize the snake's starting coordinate
    // Note: xrand() already seeds itself using the operating system's entropy source
    const size_t row_delta = xrand() % region_size.row;
    const size_t col_delta = xrand() % region_size.col;
    state->position = (GameCoord){
        .row = region_min.row + row_delta,
        .col = region_min.col + col_delta,
    };

    // Randomize the snakes direction
    const bool is_horizontal = xrand() % 2;
    
    // The snake will spawn facing away from the closest wall in its direction
    const GameCoord mid_point = {state->position_max.row / 2, state->position_max.col / 2,};
    const char* snake_body = NULL;
    if (is_horizontal)
    {
        snake_body = SNAKE_HORIZONTAL;

        if (state->position.col < mid_point.col)
        {
            state->direction = DIR_RIGHT;
        }
        else
        {
            state->direction = DIR_LEFT;
        }
    }
    else // vertical
    {
        snake_body = SNAKE_VERTICAL;
        
        if (state->position.row < mid_point.row)
        {
            state->direction = DIR_DOWN;
        }
        else
        {
            state->direction = DIR_UP;
        }
    }

    // The starting head's direction
    const char* snake_head = "?";
    switch (state->direction)
    {
        case DIR_RIGHT:
            snake_head = SNAKE_HEAD_RIGHT;
            break;
        
        case DIR_LEFT:
            snake_head = SNAKE_HEAD_LEFT;
            break;
        
        case DIR_DOWN:
            snake_head = SNAKE_HEAD_DOWN;
            break;
        
        case DIR_UP:
            snake_head = SNAKE_HEAD_UP;
            break;
        
        default:
            break;
    }

    /* Draw the snake */

    GameCoord pos = {state->position.row, state->position.col}; // Current drawing position

    // Draw the head at the starting position
    // (the snake's color is green)
    printf(MOVE_CURSOR(%zu,%zu) TEXT_GREEN "%s", pos.row, pos.col, snake_head);
    state->arena[pos.row - 1][pos.col - 1] = true;
    state->head = sid;
    state->snake[sid++] = (GameCoord){pos.row, pos.col};

    // Draw the body
    for (size_t i = 0; i < (SNAKE_START_SIZE - 1); i++)
    {
        move_coord(&pos, state->direction, -1);
        printf(MOVE_CURSOR(%zu,%zu) "%s", pos.row, pos.col, snake_body);
        state->arena[pos.row - 1][pos.col - 1] = true;
        state->snake[sid++] = (GameCoord){pos.row, pos.col};
    }
    state->tail = sid - 1;

    // Keep track of the snake's size
    state->size = SNAKE_START_SIZE;

    // Display the snake's size
    print_snake_size(state);

    // Spawn the first food pellet
    spawn_food(state);

    // Calculate the time between game ticks
    // (after the game state is updated, the program will sleep for the remaining of this time before updating it again)
    state->tick_time_start = 1000000 / SNAKE_START_SPEED;
    state->tick_time_final = 1000000 / SNAKE_FINAL_SPEED;

    // Clamp the game speed to the range [1, 12]
    if (speed < 1) speed = 1;
    if (speed > 12) speed = 12;

    /* Apply the speed modifiers */

    if (speed < 5)  // Snake goes slower than the default
    {
        const uint64_t delta_s = (state->tick_time_start * 2) - state->tick_time_start;
        state->tick_time_start += (delta_s * (5 - speed)) / 4;

        const uint64_t delta_f = (state->tick_time_final * 2) - state->tick_time_final;
        state->tick_time_final += (delta_f * (5 - speed)) / 4;
    }
    else if (speed > 5) // Snake goes faster than the default
    {
        const uint64_t delta_s = state->tick_time_start - (state->tick_time_start / 2);
        state->tick_time_start -= (delta_s * (speed - 5)) / 4;

        const uint64_t delta_f = state->tick_time_final - (state->tick_time_final / 2);
        state->tick_time_final -= (delta_f * (speed - 5)) / 4;
    }
    
    // Output the game screen to the terminal
    fflush(stdout);

    return state;
}

// MAIN LOOP: check for input and update the game state
void game_mainloop(GameState* state)
{
    // Wait for the first game frame
    wait_usec(state->tick_time_start);
    
    // The time between game ticks keeps decreasing up to this amount as the snake grows
    uint64_t max_time_mod = state->tick_time_start - state->tick_time_final;

    // How much has currently the tick timer decreased from the initial value
    uint64_t time_mod = (state->size * max_time_mod) / state->total_area;

    // Size of the snake at the beginning of the frame
    // ('time_mod' is going to be updated whenever the snake grows)
    size_t old_size = state->size;
    
    while (true)
    {
        // Keep track of how long it took to update the game state
        const uint64_t start_time = clock_usec();
        
        // Get which direction key the user has pressed
        SnakeDirection dir = parse_input(state);

        // The snake accelerates if the user has pressed the same direction the snake is moving
        const bool accelerate = (dir == state->direction);
        if (dir == DIR_NONE) dir = state->direction;

        // Move the snake in the current direction, while updating the game state accordingly.
        // Then check if the snake has collided with an wall or itself.
        const bool has_collided = move_snake(state, dir);

        // It is game over if the snake has collided or there are no more empty spaces
        if (has_collided || state->free_area == 0)
        {
            game_over(state);
            break;
        }
        
        // The snake's speed increase as it grows
        if (state->size != old_size)
        {
            time_mod = (state->size * max_time_mod) / state->total_area;
            old_size = state->size;
        }

        // Calculate how long before the next iteration of the loop
        uint64_t frame_duration = state->tick_time_start - time_mod;
        if (accelerate) frame_duration = frame_duration / SNAKE_ACCEL_FACTOR;
        
        // Wait through the remaining time
        const uint64_t elapsed_time = clock_usec() - start_time;
        if (elapsed_time < frame_duration)
        {
            wait_usec(frame_duration - elapsed_time);
        }
    }
}

// Return to the main terminal screen, print the size, and free the memory used by the game
void game_close(GameState* state)
{
    // Print the snake's size on exit
    printf(MAIN_SCREEN);
    if (state->free_area == 0)
    {
        printf(TEXT_GREEN "CONGRATULATIONS:" COLOR_RESET " Snake overflow!\n");
    }
    printf(TEXT_YELLOW "Final size:" COLOR_RESET " %zu\n", state->size);
    fflush(stdout);
    
    // Reset the terminal's properties to the original and free the allocated memory
    cleanup();
    map_destroy(state->keymap);
    free(state->arena);
    free(state->snake);
    free(state);
    state_ptr = NULL;

    #ifdef _WIN32
    // Whether virtual terminal sequences were already enabled on Windows console
    windows_vt_seq = false;
    #else
    // Whether the program has already changed the attributes of the terminal on Linux
    linux_term_flags_set = false;
    #endif // _WIN32
}
