#include "includes.h"

// Remember the game state for clean-up purposes
static GameState *state_ptr;

// Reset the terminal and its window back to their original states
static void cleanup()
{
    SetWindowLong(state_ptr->window, GWL_STYLE, state_ptr->window_mode_old);
    printf(TERM_RESET MAIN_SCREEN);
    SetConsoleMode(state_ptr->input_handle, state_ptr->input_mode_old);
    SetConsoleMode(state_ptr->output_handle, state_ptr->output_mode_old);
}

// Perform a clean-up before exiting
static _Noreturn void clean_exit()
{
    cleanup();
    exit(EXIT_SUCCESS);
}

// Allocate memory initialized to zero and check if it has been successfully allocated
// Note: program exits on failure.
static void* xmalloc(size_t size)
{
    void* ptr = calloc(1, size);
    if (!ptr)
    {
        cleanup();
        fprintf(stderr, TEXT_RED "Error:" COLOR_RESET " Not enough memory.\n");
        quick_exit(EXIT_FAILURE);
    }
    return ptr;
}

// Set-up the game state and draw the initial screen
GameState* game_init()
{
    // Reset the terminal to its default properties when the program exits
    signal(SIGINT, &clean_exit);    // Closing with Ctrl+C
    signal(SIGTERM, &clean_exit);   // Closing through task manager
    atexit(&cleanup);               // Program closed itself

    // Allocate and initialize the game state
    GameState *state = xmalloc(sizeof(GameState));
    state_ptr = state;

    #ifdef _WIN32

    // Make the terminal to accept Unicode characters encoded in UTF-8
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

    #else // Linux


    #endif // _WIN_32

    // Switch to the alternate terminal screen.
    // Then make its background black, its text light grey, and enable keyboard input.
    printf(ALT_SCREEN BG_BLACK TEXT_GRAY CLEAR_SCREEN KP_APP_MODE CK_APP_MODE);
}
