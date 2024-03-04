#pragma once

#include "includes.h"

// Remember the game state for clean-up purposes
extern GameState *state_ptr;

#ifdef _WIN32
// Whether virtual terminal sequences were already enabled on Windows console
extern bool windows_vt_seq;
#else
// Whether the program has already changed the attributes of the terminal on Linux
extern bool linux_term_flags_set;
#endif // _WIN32

// Reset the terminal and its window back to their original states
void cleanup(void);

// Signal handler for segmentation fault: reset the terminal then exit.
// This is for preventing the terminal from remaining on the alternate screen with the changed terminal flags.
// Needless to say, if this function ends up being called it means there are errors in the code that must be fixed.
void _Noreturn exit_segfault(int signal);

// Prints a formatted string as an error then exit the program with the given status code
// Note: "Error: " (in red) is added before the message, and a line break is added after the message.
void _Noreturn printf_error_exit(int status_code, const char* format, ...);

#ifdef _WIN32
// On Windows, print the error message from GetLastError() then exit the program returning its error code
// Note: the message is prefixed with the name of the source file and the line number of where this function was called from.
void _Noreturn windows_error_exit(const char* file_name, int line_number);
#endif // _WIN32

// Allocate memory initialized to zero and check if it has been successfully allocated
// Note: program exits on failure.
void* xmalloc(size_t size);

// Allocate memory for a two dimensional array and initialize its elements to zero
// Layout: array[height][width]
// Note: the returned pointer can be passed to free() to get the entire array freed at once.
void** alloc_2Darray(size_t width, size_t height, size_t element_size);

// Move in-place a coordinate by a certain offset in the given direction
inline void move_coord(GameCoord *coord, SnakeDirection dir, size_t offset);

// Generate a pseudo-random unsigned integer from 0 to UINT_MAX
// Note: this function uses the entropy source from the operating system, so seeding is not needed.
unsigned int xrand();

// Check if there is any input to be read from stdin
bool input_available();

// Get a character from stdin without blocking if there is no input available
// (return EOF in such case)
int getchar_nb();

// Get the amount of microseconds since an unespecified point of time
// The difference between two calls of this function should give how long has passed.
uint64_t clock_usec();

// Return after the given amount of microseconds
void wait_usec(uint64_t time);

// Reset the terminal size to the original values
// Note: this function is meant to be called when the terminal window is resized on Linux (SIGWINCH signal)
void restore_term(int signal);

// Discard all the data remaining on the standard input stream
void flush_stdin();

// Convert a null terminated string to an unsigned integer
// Note: On success it returns 'true' and stores the result on 'out'.
//       The string must end after the last digit, and start with a digit or blank spaces.
bool parse_uint(const char* string, unsigned int* out);

// Parse the speed value from the program's arguments
// Note: program exits on failure
unsigned int parse_speed(int argc, char** argv);

// (debugging) Save the current collision grid to a text file
void save_collision_grid(GameState *state, const char* path);

// (debugging) Print the corresponding arrow when a direction key is pressed
void debug_keys();

// (debugging) Move the snake freely with the keyboard
void debug_movement();
