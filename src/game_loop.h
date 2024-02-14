#pragma once

#include "includes.h"

#define SCREEN_MARGIN 1     // Distance that the game box is drawn from the terminal's borders
#define SNAKE_START_SIZE 4  // Initial size of the snake

typedef struct GameState GameState;
typedef struct GameCoord GameCoord;
typedef enum SnakeDirection {DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_LEFT} SnakeDirection;

// Coordinates on the terminal
// Notes: 1-indexed, top left is (1,1).
struct GameCoord
{
    size_t row; // Row number
    size_t col; // Column number
};

// Information needed for drawing the game
// IMPORTANT: all screen coordinates are 1-indexed, because the numbering of the terminal's rows and columns also start at 1.
struct GameState
{
    bool **arena;               // (2D array) Collision grid for the game area ('true' means a occupied space)
    GameCoord *snake;           // (double-ended queue) Coordinates of the screen where the snake parts are being drawn
    size_t head;                // Index of .snake[] where the head is
    size_t tail;                // Index of .snake[] where the tail is
    GameCoord screen_size;      // Maximum coordinates on the terminal screen
    GameCoord position;         // Coordinate of the screen where the snake's head is
    GameCoord position_min;     // Smallest screen coordinate where the snake's head can go
    GameCoord position_max;     // Biggest screen coordinate where the snake's head can go
    GameCoord food;             // Screen coordinate of the food pellet
    SnakeDirection direction;   // Direction the snake is moving to
    uint32_t sleep_time;        // Duration of each drawn frame
    uint64_t score;             // Player's score

    #ifdef _WIN32
    // Handles and modes for the terminal on Windows
    HANDLE input_handle;    // stdin handle
    DWORD input_mode;       // stdin settings
    DWORD input_mode_old;   // original stdin settings
    HANDLE output_handle;   // stdout handle
    DWORD output_mode;      // stdout settings
    DWORD output_mode_old;  // original stdout settings
    HWND window;            // terminal's window handle
    LONG window_mode;       // terminal's window settings
    LONG window_mode_old;   // original terminal's window settings
    UINT output_cp_old;     // original code page of the terminal
    
    #else
    struct termios term_flags_old;  // original settings of the terminal
    
    #endif // _WIN32
};

// Set-up the game state and draw the initial screen
GameState* game_init();
