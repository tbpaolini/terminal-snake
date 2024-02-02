#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>

#else
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

#endif // _WIN_32
