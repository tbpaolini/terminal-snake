#pragma once

#include "includes.h"

#define SCREEN_MARGIN 1     // Distance that the game box is drawn from the terminal's borders
#define SNAKE_START_SIZE 4  // Initial size of the snake
#define SNAKE_START_SPEED 3     // Snake's speed at the start of the game (unit: spaces per second)
#define SNAKE_FINAL_SPEED 15    // Snake's speed can increase up to this value as the game progresses
#define SNAKE_ACCEL_FACTOR 2    // Snake's speed is multiplied by this value when pressing the direction the snake is facing
#define SLEEP_MARGIN 15000  // Program wakes up this amount of microseconds before the start of the next frame

// Keyboard's scan codes to be mapped to a direction
#define SCANCODE_UP    17   // `W` key when on a QWERTY keyboard
#define SCANCODE_LEFT  30   // `A` key when on a QWERTY keyboard
#define SCANCODE_DOWN  31   // `S` key when on a QWERTY keyboard
#define SCANCODE_RIGHT 32   // `D` key when on a QWERTY keyboard

typedef struct GameState GameState;
typedef struct GameCoord GameCoord;
typedef struct KeyMap KeyMap;
typedef enum SnakeDirection {DIR_NONE=0, DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_LEFT} SnakeDirection;

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
    KeyMap *keymap;             // Character keys mapped to the directions (can be used in addition to the arrow keys)
    size_t head;                // Index of .snake[] where the head is
    size_t tail;                // Index of .snake[] where the tail is
    size_t size;                // Current size of the snake
    size_t total_area;          // Total amount of spaces inside the snake's area
    size_t free_area;           // Count of non-blocking spaces on the snake's area
    GameCoord screen_size;      // Maximum coordinates on the terminal screen
    GameCoord position;         // Coordinate of the screen where the snake's head is
    GameCoord position_min;     // Smallest screen coordinate where the snake's head can go
    GameCoord position_max;     // Biggest screen coordinate where the snake's head can go
    GameCoord food;             // Screen coordinate of the food pellet
    SnakeDirection direction;   // Direction the snake is moving to
    uint64_t tick_time_start;   // Duration (in microseconds) at the game's start for each drawn frame 
    uint64_t tick_time_final;   // The sleep time can decrease up to this value as the game progresses

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
// 'speed' is a value from 1 to 12 for setting the initial snake's speed.
// 5 is the default speed. 1 is half of the default and 9 is double of the default.
// Other values are a linear interpolation between those points.
GameState* game_init(unsigned int speed);

// MAIN LOOP: check for input and update the game state
void game_mainloop(GameState* state);

// Return to the main terminal screen, print the size, and free the memory used by the game
void game_close(GameState* state);
