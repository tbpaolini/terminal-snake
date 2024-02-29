#pragma once

#ifdef _WIN32
#define _CRT_RAND_S /* Enable the 'rand_s()' function on Windows */
    #ifdef _MSC_VER
    // Macros to "enable" some keywords that the MSVC compiler does not support
    #define typeof __typeof__
    #define _Noreturn __declspec(noreturn)
    #endif  // _MSC_VER
#endif // _WIN32

// Standard headers
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <locale.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

// OS headers
#ifdef _WIN32
#include <windows.h>
#include <conio.h>

#else

#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>

// Check if the '__fpurge()' function is available
// (it should be available on a GNU C Library version 2.2 or later, in the <stdio_ext.h> header)
#if defined(__GLIBC__) && defined(__GLIBC_PREREQ)
    #if __GLIBC_PREREQ(2,2)
    #define __FPURGE_AVAILABLE
    #include <stdio_ext.h>
    #endif // __GLIBC_PREREQ
#endif // __GLIBC__

#endif // _WIN_32

// Application's headers
#include "terminal_sequences.h"
#include "screen_elements.h"
#include "game_loop.h"
#include "game_logic.h"
#include "helper_functions.h"
#include "helper_macros.h"

// Error codes
#define ERR_NO_MEMORY -1        // Failed to allocate memory
#define ERR_TINY_TERMINAL -2    // Terminal window is too small for the game
#define ERR_ARRAY_OVERFLOW -3   // Tried to access an out-of-bounds coordinate
#define ERR_INVALID_ARGS -4     // Incorrect command line arguments passed to the program
