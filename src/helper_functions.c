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
void cleanup(void)
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

// Signal handler for segmentation fault: reset the terminal then exit.
// This is for preventing the terminal from remaining on the alternate screen with the changed terminal flags.
// Needless to say, if this function ends up being called it means there are errors in the code that must be fixed.
void _Noreturn exit_segfault(int signal)
{
    cleanup();
    state_ptr = NULL;
    fprintf(stderr, "Segmentation fault\n");
    _Exit(signal);
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
    const size_t buffer_size = (sizeof(error_message) / sizeof(WCHAR)) - 1; // Leave space for a NUL terminator at the end

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
    if (size == 0)
    {
        printf_error_exit(ERR_INVALID_ARGS, "Tried to allocate zero bytes of memory.");
    }
    
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
    if (width == 0 || height == 0)
    {
        printf_error_exit(ERR_INVALID_ARGS, "2D array must not have an width or a height of zero.");
    }
    
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
extern inline void move_coord(GameCoord *coord, SnakeDirection dir, size_t offset)
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
        
        default:
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
        // Seed the pseudo-random number generator on Linux
        // Note: On Windows it is seeded automatically.
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

    if (kbhit()) return true;
    /* Note: I have tried quite a few approaches using the modern Win32 API,
       but none fully worked and they were quite cumbersome.
       Fortunately the old <conio.h> from MS DOS days came to the rescue! :-)
    */

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

// Get the amount of microseconds since an unespecified point of time
// The difference between two calls of this function should give how long has passed.
uint64_t clock_usec()
{
    #ifdef _WIN32
    // At the first time this function runs, get the amount of counts per second
    static bool got_freq = false;
    static LARGE_INTEGER freq;
    if (!got_freq)
    {
        WINDOWS_ERROR_CHECK(QueryPerformanceFrequency(&freq));
        got_freq = true;
    }

    // Get the amount of counts
    LARGE_INTEGER counter;
    WINDOWS_ERROR_CHECK(QueryPerformanceCounter(&counter));

    // Calculate and return the amount of microseconds
    return (counter.QuadPart * 1000000) / freq.QuadPart;
    
    #else // Linux
    
    // Get the time in nanoseconds
    struct timespec counter;
    LINUX_ERROR_CHECK(clock_gettime(CLOCK_MONOTONIC, &counter));

    // Calculate and return the amount of microseconds
    return ((counter.tv_sec * 1000000000) + counter.tv_nsec) / 1000;
    
    #endif
}

// Return after the given amount of microseconds
void wait_usec(uint64_t time)
{
    uint64_t start = clock_usec();

    if (time > SLEEP_MARGIN)
    {
        #ifdef _WIN32
        SleepEx( (time - SLEEP_MARGIN) / 1000, FALSE );
        #else // Linux
        usleep(time - SLEEP_MARGIN);
        #endif
    }
    
    while (clock_usec() - start < time) continue;
}

// Reset the terminal size to the original values
// Note: this function is meant to be called when the terminal window is resized on Linux (SIGWINCH signal)
void restore_term(int signal)
{
    if (!state_ptr) return;
    printf(RESIZE_SCREEN(%zu,%zu) HIDE_CURSOR, state_ptr->screen_size.row, state_ptr->screen_size.col);
    fflush(stdout);
}

// Discard all the data remaining on the standard input stream
void flush_stdin()
{
    #ifdef _WIN32
    FlushConsoleInputBuffer(state_ptr->input_handle);
    #else
    tcflush(STDIN_FILENO, TCIFLUSH);
    #endif
}

// Convert a null terminated string to an unsigned integer
// Note: On success it returns 'true' and stores the result on 'out'.
//       The string must end after the last digit, and start with a digit or blank spaces.
bool parse_uint(const char* string, unsigned int* out)
{
    if (*string == '\0') return false;
    
    char* end = NULL;
    unsigned int value = strtoul(string, &end, 10);
    
    if (end && *end == '\0')
    {
        *out = value;
        return true;
    }
    
    return false;
}

// Parse the speed value from the program's arguments
// Note: program exits on failure
unsigned int parse_speed(int argc, char** argv)
{
    if (argc > 1)
    {
        if (argc == 2)
        {
            unsigned int out;
            if (parse_uint(argv[1], &out))
            {
                if (out >= 1 && out <= 12) return out;
                else goto fail;
            }
            else goto fail;
        }
        else
        {
            fail:
            printf(
                "Please pass a value from 1 to 12 as the only argument to this program in order to set the game's speed.\n\n"
                "Example:\n\t%s 10\n\n"
                "The greater the value, the faster the snake moves.\n"
                "5 is the default speed. 1 is half of it, 9 is twice it.\n\n"
                "This game was programmed by Tiago Becerra Paolini, and is licensed under the MIT License.\n\n"
                "Author's e-mail: tpaolini@gmail.com\n"
                "Source code: https://github.com/tbpaolini/terminal-snake\n"
                "Version: 1.0.5\n"
                "Built on: " __DATE__ " " __TIME__ "\n",
                argv[0]
            );
            exit(ERR_INVALID_ARGS);
        }
    }
    
    return 5;
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
        SnakeDirection dir = parse_input(state_ptr);
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
        SnakeDirection dir = parse_input(state_ptr);
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
