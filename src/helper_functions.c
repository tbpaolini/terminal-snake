#include "includes.h"

// Remember the game state for clean-up purposes
GameState *state_ptr = NULL;

#ifdef _WIN32
// Whether virtual terminal sequences were already enabled on Windows console
bool windows_vt_seq = false;
#else
// Whether the program has already changed the attributes of the terminal on Linux
bool linux_term_flags_set = false;
#endif // _WIN32

// Reset the terminal and its window back to their original states
void cleanup()
{
    // Exit the game screen then return to the main terminal screen
    #ifdef _WIN32
    if (windows_vt_seq) printf(TERM_RESET MAIN_SCREEN);
    #else
    printf(TERM_RESET MAIN_SCREEN);
    #endif // _WIN32
    fflush(stdout);

    // Reset the terminal's settings back to the original
    if (!state_ptr) return;
    #ifdef _WIN32
    SetWindowLong(state_ptr->window, GWL_STYLE, state_ptr->window_mode_old);
    SetConsoleOutputCP(state_ptr->output_cp_old);
    SetConsoleMode(state_ptr->input_handle, state_ptr->input_mode_old);
    SetConsoleMode(state_ptr->output_handle, state_ptr->output_mode_old);
    windows_vt_seq = false;

    #else // Linux
    if (linux_term_flags_set) tcsetattr(STDIN_FILENO, TCSANOW, &state_ptr->term_flags_old);

    #endif // _WIN32
}

// Prints a formatted string as an error then exit the program with the given status code
// Note: "Error: " (in red) is added before the message, and a line break is added after the message.
void _Noreturn printf_error_exit(int status_code, const char* format, ...)
{
    // Exit the game screen then return to the main terminal screen
    cleanup();
    state_ptr = NULL;
    
    // Print the red text "Error:"
    #ifdef _WIN32
    if (windows_vt_seq) fprintf(stderr, TEXT_RED "Error: " COLOR_RESET);
    else fprintf(stderr, "Error: ");
    
    #else
    fprintf(stderr, TEXT_RED "Error: " COLOR_RESET);
    
    #endif // _WIN32
    
    // Print the formatted string to stderr
    va_list args = {0};
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    
    // Close the program and return the error code
    exit(status_code);
}

#ifdef _WIN32
// On Windows, print the error message from GetLastError() then exit the program returning its error code
// Note: the message is prefixed with the name of the source file and the line number of where this function was called from.
void _Noreturn windows_error_exit(const char* file_name, int line_number)
{
    // Get from the last error code of the Windows API
    const DWORD error_code = GetLastError();
    WCHAR error_message[1024] = {0};
    const size_t buffer_size = sizeof(error_message) - sizeof(WCHAR);   // Leave space for a NUL terminator at the end

    // Get the text that describes the error
    DWORD message_size = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM |    // Get the message from the system's string table
        FORMAT_MESSAGE_IGNORE_INSERTS,  // Ignore insert sequences (like %1) on the string
        NULL,                           // String table (NULL since we are using the one from the system)
        error_code,                     // Value returned by GetLastError()
        0,                              // Language ID (0 for using the system's current language)
        (LPWSTR)error_message,          // Buffer to store the error message
        buffer_size,                    // Size of the message buffer
        NULL                            // Format string for the message (NULL since we are ignoring insert sequences)
    );

    // Exit the game screen then return to the main terminal screen
    cleanup();
    state_ptr = NULL;

    // Ensure that non-english characters are going to be printed properly 
    setlocale(LC_ALL, "");
    
    // Print the error message to stderr then exit returning the error code
    fwprintf(stderr, L"Error %lu at [%hs:%d]: %s", error_code, file_name, line_number, error_message);
    exit(error_code);
}
#endif // _WIN32

// Allocate memory initialized to zero and check if it has been successfully allocated
// Note: program exits on failure.
void* xmalloc(size_t size)
{
    void* ptr = calloc(1, size);
    if (!ptr)
    {
        printf_error_exit(ERR_NO_MEMORY, "Not enough memory.");
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
        size_t read_count = 0;
        if (dev_urandom)
        {
            read_count = fread(&seed, sizeof(seed), 1, dev_urandom);
            fclose(dev_urandom);
        }
        if (read_count != 1)
        {
            printf_error_exit(
                errno,
                "Could not seed the pseudo-random number generator with bytes from '/dev/urandom' (%s).",
                strerror(errno)
            );
        }
        srandom(seed);
        is_seeded = true;
    }
    return random();
    
    #endif
}

// Check if there is any input to be read from stdin
bool input_available()
{
    #ifdef _WIN32

    DWORD input_count = 0;
    WINDOWS_ERROR_CHECK(GetNumberOfConsoleInputEvents(state_ptr->input_handle, &input_count));
    if (input_count > 0) return true;

    #else
    fd_set descriptors = {0};
    FD_ZERO(&descriptors);
    FD_SET(STDIN_FILENO, &descriptors);
    struct timeval timeout = {0};
    const int status = select(STDIN_FILENO+1, &descriptors, NULL, NULL, &timeout);
    if (status == 1) return true;

    #endif // _WIN32

    return false;
}

// Get a character from stdin without blocking if there is no input available
// (return EOF in such case)
int getchar_nb()
{
    if ( input_available() )
    {
        return getchar();
    }
    else
    {
        return EOF;
    }
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
            if ((row+1 == state->food.row) && (col+1 == state->food.col))
            {
                fprintf(f, "*");    // Show the food pellet's position
            }
 
            fprintf(f, "%d", state->arena[row][col]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

// (debugging) Print the corresponding arrow when a direction key is pressed
void debug_keys()
{
    while (true)
    {
        SnakeDirection dir = parse_input();
        switch (dir)
        {
            case DIR_UP:
                printf(u8"↑");
                fflush(stdout);
                break;
            
            case DIR_DOWN:
                printf(u8"↓");
                fflush(stdout);
                break;
            
            case DIR_LEFT:
                printf(u8"←");
                fflush(stdout);
                break;
            
            case DIR_RIGHT:
                printf(u8"→");
                fflush(stdout);
                break;
            
            default:
                break;
        }
    }
}

// (debugging) Move the snake freely with the keyboard
void debug_movement()
{
    while (true)
    {
        SnakeDirection dir = parse_input();
        if (dir)
        {
            move_snake(state_ptr, dir);
        }

        // Sleep for 5 milliseconds before checking for input again
        #ifdef _WIN32
        Sleep(5);
        #else
        usleep(5000);
        #endif
    }
}
