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

// Allocate memory for a two dimensional array and initialize its elements to zero
// Layout: array[height][width]
// Note: the returned pointer can be passed to free() to get the entire array freed at once.
void** alloc_2Darray(size_t width, size_t height, size_t element_size)
{
    const size_t col_size = sizeof(void*) * height;
    const size_t row_size = width * element_size;
    const size_t total_size = col_size + (height * row_size);

    // The 2D array is a sequence of row pointers followed by the elements themselves
    void** array = xmalloc(total_size);
    
    // Add the row pointers to the array
    uintptr_t row_address = (uintptr_t)array + col_size;
    for (size_t i = 0; i < height; i++)
    {
        array[i] = (void*)row_address;
        row_address += row_size;
    }
    
    return array;
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
    if (!state_ptr) return;
    printf(RESIZE_SCREEN(%zu,%zu), state_ptr->screen_size.row, state_ptr->screen_size.col);
    fflush(stdout);
}

// (debugging) Save the current collision grid to a text file
void save_collision_grid(GameState *state, const char* path)
{
    FILE *f = fopen(path, "wt");
    for (size_t row = 0; row < state->screen_size.row; row++)
    {
        for (size_t col = 0; col < state->screen_size.col; col++)
        {
            fprintf(f, "%d", state->arena[row][col]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}
