#pragma once

/*
Note:
    These macros are meant to be passed as part of the strings given to console printing functions.
   
Example:
    printf(ALT_SCREEN CLEAR_SCREEN MOVE_CURSOR(%d, %d) "Hello world!", 10, 5)
    (This will switch to an empty terminal screen, then print the text to row=10 column=5)
*/

// Control Sequence Introducer: 'ESC' (0x1B) followed by '[' (left bracket, 0x5B)
#define CSI "\x1b["

// Control of the terminal screen
#define ALT_SCREEN CSI "?1049h"     // Switch to the alternate terminal screen
#define MAIN_SCREEN CSI "?1049h"    // Switch back to the main terminal screen
#define CLEAR_SCREEN CSI "2J"       // Erase all characters on the terminal screen

// Control of the cursor
#define MOVE_CURSOR(row,column) CSI #row";"#column"H"   // Move the text cursor to a given row and column (1-indexed)
#define HIDE_CURSOR CSI "?25l"      // Do not display the text cursor
#define SHOW_CURSOR CSI "?25h"      // Display the text cursor

// Text and background colors
// More color values at: https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#text-formatting
#define COLOR_RESET CSI "0m"        // Reset text and background colors back to their defaults
#define TEXT_GREEN CSI "92m"
#define TEXT_YELLOW CSI "93m"
#define TEXT_CYAN CSI "96m"
#define TEXT_GRAY CSI "37m"
#define TEXT_WHITE CSI "97m"
#define BG_BLACK CSI "40m"

// Reset certain terminal properties back to the default values
// (cursor visibility, numeric keypad, cursor keys mode, top and bottom margins, character set,
//  colors, text formatting, saved cursor state)
#define TERM_RESET CSI "!p"
