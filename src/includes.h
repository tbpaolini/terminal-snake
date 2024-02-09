#pragma once

// Standard headers
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <locale.h>
#include <signal.h>

// OS headers
#ifdef _WIN32
#include <windows.h>

#else
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

#endif // _WIN_32

// Application's headers
#include "terminal_sequences.h"
#include "screen_elements.h"
#include "game_loop.h"
