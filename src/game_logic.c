#include "includes.h"

// Reset the terminal and its window back to their original states
static void cleanup()
{
    printf(TERM_RESET MAIN_SCREEN);
}

// Perform a clean-up before exiting
static void clean_exit()
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
