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
    tcsetattr(STDIN_FILENO, TCSANOW, &state_ptr->term_flags_old);

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

// Generate a pseudo-random unsigned integer from 0 to UINT_MAX (on Windows) or 2^31 - 1 (on Linux)
// Note: this function uses the entropy source from the operating system, so seeding is not needed.
unsigned int xrand()
{
    #ifdef _WIN32
    unsigned int out = 0;
    rand_s(&out);   // Note: this function does not need manual seeding
    return out;
    
    #else // Linux
    static bool is_seeded = false;
    if (!is_seeded)
    {
        FILE* dev_urandom = fopen("/dev/urandom", "rb");
        unsigned int seed = 0;
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-result"
        fread(&seed, sizeof(seed), 1, dev_urandom);
        #pragma GCC diagnostic pop
        fclose(dev_urandom);
        srandom(seed);
        is_seeded = true;
    }
    return random();
    
    #endif
}

// Reset the terminal size to the original values
// Note: this function is meant to be called when the terminal window is resized on Linux (SIGWINCH signal)
void restore_term(int signal)
{
    printf(RESIZE_SCREEN(%zu,%zu), state_ptr->screen_size.row, state_ptr->screen_size.col);
    fflush(stdout);
}
