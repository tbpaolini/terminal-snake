#include "includes.h"

// Set-up the game state and draw the initial screen
GameState* game_init()
{
    // Reset the terminal to its default properties when the program exits
    atexit(&cleanup);         // Run our clean-up routine on exit
    signal(SIGINT, &exit);    // Closing with Ctrl+C
    signal(SIGTERM, &exit);   // Closing through task manager

    // Allocate and initialize the game state
    GameState *state = xmalloc(sizeof(GameState));
    state_ptr = state;

    #ifdef _WIN32

    // Make the terminal to accept Unicode characters encoded in UTF-8
    state->output_cp_old = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);

    // Get the Windows' handles for the standard input and output streams
    state->output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    state->input_handle = GetStdHandle(STD_INPUT_HANDLE);

    // Get the settings of the output stream
    if (!GetConsoleMode(state->output_handle, &state->output_mode))
    {
        // TO DO: Exit...
    }
    state->output_mode_old = state->output_mode;

    // Get the settings of the input stream
    if (!GetConsoleMode(state->input_handle, &state->input_mode))
    {
        // TO DO: Exit...
    }
    state->input_mode_old = state->input_mode;

    // Get the style of the window where the terminal is
    state->window = GetConsoleWindow();
    if( !(state->window_mode = GetWindowLong(state->window, GWL_STYLE)) )
    {
        // TO DO: Exit...
    }
    state->window_mode_old = state->window_mode;

    // Enable escape sequences on the terminal,
    // and do not move to the next line when writing a character to the end of a line
    state->output_mode |= (ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);
    if (!SetConsoleMode(state->output_handle, state->output_mode))
    {
        // TO DO: Exit...
    }

    // Enable input through the arrows on the terminal.
    // Also make so new characters overwrite existing ones,
    // the input is sent immediately after a key is pressed,
    // and ignore mouse input.
    state->input_mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
    state->input_mode &= (~ENABLE_INSERT_MODE & ~ENABLE_LINE_INPUT & ~ENABLE_MOUSE_INPUT);
    if (!SetConsoleMode(state->input_handle, state->input_mode))
    {
        // TO DO: Exit...
    }
    
    // For the terminal's window, disable: maximizing button, resizing, horizontal and vertical scrollbars
    state->window_mode &= (~WS_MAXIMIZEBOX & ~WS_SIZEBOX & ~WS_HSCROLL & ~WS_VSCROLL);
    if (!SetWindowLong(state->window, GWL_STYLE, state->window_mode))
    {
        // TO DO: Exit...
    }

    // Get the amount of rows and columns that are visible on the terminal window
    CONSOLE_SCREEN_BUFFER_INFO buffer_info = {0};
    GetConsoleScreenBufferInfo(state->output_handle, &buffer_info);
    state->screen_size = (GameCoord){
        buffer_info.srWindow.Bottom - buffer_info.srWindow.Top + 1,
        buffer_info.srWindow.Right - buffer_info.srWindow.Left + 1,
    };

    #else // Linux

    // Disable input echoing and make the input to be available immediately
    // (that is, no need for a line to be entered in order for us to read the user's input)
    struct termios term_flags = {0};
    tcgetattr(STDIN_FILENO, &term_flags);
    state->term_flags_old = term_flags;
    term_flags.c_lflag &= (~ECHO & ~ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &term_flags);

    // Get the amount of rows and columns that are visible on the terminal window
    struct winsize term_size = {0};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &term_size);
    state->screen_size = (GameCoord){
        term_size.ws_row,
        term_size.ws_col,
    };

    // Change the terminal back to the original size when the window is resized
    signal(SIGWINCH, &resize_term);

    #endif // _WIN_32

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

    // Set the output stream to fully buffered so it is only drawn when we flush it
    setvbuf(stdout, NULL, _IOFBF, state->screen_size.col * state->screen_size.row * 4);
    
    /* Drawing a rectangle along the terminal's borders */
    
    // Draw the top border
    printf(MOVE_CURSOR(%zu, %zu) BOX_TOP_LEFT, board_start.row, board_start.col);
    for (size_t i = 0; i < state->screen_size.col - 2 * (SCREEN_MARGIN + 1); i++)
    {
        printf(BOX_HORIZONTAL);
    }
    printf(BOX_TOP_RIGHT);
    
    // Draw the laterals
    for (size_t i = board_start.row + SCREEN_MARGIN; i <= board_end.row; i++)
    {
        printf(MOVE_CURSOR(%zu, %zu) BOX_VERTICAL MOVE_CURSOR(%zu, %zu) BOX_VERTICAL, i, board_start.col, i, board_end.col);
    }

    // Draw the bottom border
    printf(MOVE_CURSOR(%zu, %zu) BOX_BOTTOM_LEFT, board_end.row, board_start.col);
    for (size_t i = 0; i < state->screen_size.col - 2 * (SCREEN_MARGIN + 1); i++)
    {
        printf(BOX_HORIZONTAL);
    }
    printf(BOX_BOTTOM_RIGHT);

    /* Snake spawning */

    // Distance from the borders of the window in which the snake may not spawn
    const size_t safety_distance = SCREEN_MARGIN + SNAKE_START_SIZE + 2;

    // Region in which the snake may spawn
    state->position_min = (GameCoord){
        .row = safety_distance,
        .col = safety_distance,
    };
    state->position_max = (GameCoord){
        .row = state->screen_size.row - safety_distance,
        .col = state->screen_size.col - safety_distance,
    };
    const GameCoord region_size = {
        .row = state->position_max.row - state->position_min.row,
        .col = state->position_max.col - state->position_min.col,
    };

    // Randomize the snake's starting coordinate
    const size_t row_delta = xrand() % region_size.row;
    const size_t col_delta = xrand() % region_size.col;
    state->position = (GameCoord){
        .row = state->position_min.row + row_delta,
        .col = state->position_min.col + col_delta,
    };

    // Randomize the snakes direction
    const bool is_horizontal = xrand() % 2;
    
    // The snake will spawn facing away from the closest wall in its direction
    const GameCoord mid_point = {state->position_max.row / 2, state->position_max.col / 2,};
    const char* snake_body = "?";
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
    }

    /* Draw the snake */

    GameCoord pos = {state->position.row, state->position.col}; // Current drawing position

    // Draw the head at the starting position
    // (the snake's color is green)
    printf(MOVE_CURSOR(%zu,%zu) TEXT_GREEN "%s", pos.row, pos.col, snake_head);

    // Draw the body
    for (size_t i = 0; i < (SNAKE_START_SIZE - 1); i++)
    {
        move_coord(&pos, state->direction, -1);
        printf(MOVE_CURSOR(%zu,%zu) "%s", pos.row, pos.col, snake_body);
    }
    
    // Output the game screen to the terminal
    fflush(stdout);
}
