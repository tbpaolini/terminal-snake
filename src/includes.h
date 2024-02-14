#pragma once

#ifdef _WIN32
#define _CRT_RAND_S /* Enable the 'rand_s()' function on Windows */
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

// OS headers
#ifdef _WIN32
#include <windows.h>

#else
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

#endif // _WIN_32

// Application's headers
#include "terminal_sequences.h"
#include "screen_elements.h"
#include "game_loop.h"
#include "game_logic.h"
#include "helper_functions.h"
