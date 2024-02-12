#include "includes.h"

// Remember the game state for clean-up purposes
GameState *state_ptr = NULL;

// Reset the terminal and its window back to their original states
void cleanup()
{
    printf(TERM_RESET MAIN_SCREEN);
    fflush(stdout);

    #ifdef _WIN32
    if (!state_ptr) return;
    SetWindowLong(state_ptr->window, GWL_STYLE, state_ptr->window_mode_old);
    SetConsoleOutputCP(state_ptr->output_cp_old);
    SetConsoleMode(state_ptr->input_handle, state_ptr->input_mode_old);
    SetConsoleMode(state_ptr->output_handle, state_ptr->output_mode_old);

    #else // Linux

    #endif // _WIN32

}

// Allocate memory initialized to zero and check if it has been successfully allocated
// Note: program exits on failure.
void* xmalloc(size_t size)
{
    void* ptr = calloc(1, size);
    if (!ptr)
    {
        cleanup();
        fprintf(stderr, TEXT_RED "Error:" COLOR_RESET " Not enough memory.\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// Move in-place a coordinate by a certain offset in the given direction
void move_coord(GameCoord *coord, SnakeDirection dir, size_t offset)
{
    if (!coord) return;
    switch (dir)
    {
        case DIR_UP:
            coord->row -= offset;
            break;
        
        case DIR_DOWN:
            coord->row += offset;
            break;
        
        case DIR_RIGHT:
            coord->col += offset;
            break;
        
        case DIR_LEFT:
            coord->col -= offset;
            break;
    }
}
