#pragma once

#include "includes.h"

typedef struct GameState GameState;
typedef struct GameCoord GameCoord;
typedef enum SnakeDirection {UP, DOWN, RIGHT, LEFT} SnakeDirection;
typedef enum GameArena {FOOD=-1, EMPTY=0, BLOCKED=1} GameArena;

// Information needed for drawing the game
struct GameState
{
    GameArena **arena;          // (2D array) Elements on each coordinate of the screen 
    GameCoord *snake;           // (double-ended queue) Coordinates of the screen where the snake parts are being drawn
    size_t head;                // Index of snake[] where the head is
    size_t tail;                // Index of snake[] where the tail is
    GameCoord position;         // Coordinate of the screen where the snake's head is
    SnakeDirection direction;   // Direction the snake is moving to
    uint32_t sleep_time;        // Duration of each drawn frame
    uint64_t score;             // Player's score
};

// Coordinates on the terminal
// Notes: 1-indexed, top left is (1,1).
struct GameCoord
{
    size_t row; // Row number
    size_t col; // Column number
};
