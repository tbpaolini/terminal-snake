/* Rationale:
    The goal of this translation unit is to automatically map keys in an way that is independent from the keyboard layout.
    If we just mapped the "WASD" keys as the directional keys, that would only work in QWERTY keyboards (like US English).
    But if we map the key's scan codes instead, then the key's position should be the same regardless of the keyboard layout.
    For example, what would be "WASD" in QWERTY keyboards becomes "ZQSD" in AZERTY or ",AOE" in Dvorak.

    Note: The arrow keys can still be used in addition to the "WASD"-style keys mapped by this translation unit.
*/

#pragma once

#include "includes.h"

#define CHARBUFFER_SIZE (8)

// Sequence of 8 bytes for stroring an encoded character
typedef uint8_t CharBuffer[CHARBUFFER_SIZE];

// A trie (prefix tree) for holding the byte sequences of the mapped keyboard keys
// The scan codes are going to be converted to their corresponding byte sequences
// on the current keyboard layout, then stored in this data structure.
typedef struct KeyMap{
    SnakeDirection dir; // Direction (DIR_NONE if we aren't on a terminator node, otherwise it stores the mapped direction)
    struct KeyMap* next[256];   // Pointer to the next node of the trie (the index is the next byte on the sequence)
} KeyMap;

// Map scan codes from the keyboard to directions
// The use of this function is to map physical keys in an way that it is independent of the keyboard layout.
// The scan codes for the WASD keys of a QWERTY keyboard are (respectively): 17, 30, 31, 32.
// Note 1: if this function fails to map a key, then it defaults to map the corresponding WASD character for that key.
// Note 2: the arrow keys are hardcoded and won't me remapped by this function.
// Note 3: the returned key map should be freed with `map_destroy()`.
KeyMap* map_scancodes(uint32_t up, uint32_t left, uint32_t down, uint32_t right);

// Free the memory of a KeyMap object
void map_destroy(KeyMap* data);

/***** OS specific functions *****/

#ifdef _WIN32

// Take an array of scancode values and output an array of the corresponding multibyte characters (lowercase and uppercase)
// Each of the arrays for storing the characters  and their sizes must have at least
// twice the amount of elements than the scancode array. An size of zero for the
// output character means that conversion failed for the corresponding scancode.
// Function returns `true` if all pointers are not NULL and the sizes are big enough, otherwise returns `false`.
bool scancodes_to_mbchar(
    const uint32_t *restrict in_scancode,   // Array of scancode values
    size_t in_count,                        // Amount of elements in the scancode array
    CharBuffer *restrict out_char,          // Array to store the characters encoded in the console's code page
    uint8_t *restrict out_char_size,        // Array to store the sizes in bytes of each encoded character
    size_t out_count                        // Amount of elements in each of the two output arrays
);

#else

// Convert a keysym to the code point of its character on the Unicode table
// If successful, return 'true' and write the value to '*codepoint'. Otherwise, return 'false'.
bool keysym_to_codepoint(uint32_t keysym, uint32_t* codepoint);

// Get the keysyms corresponding to the given scancodes using the xmodmap command
// If successful, return 'true' and write the values to '*out_keysym'. Otherwise, return 'false'.
// Function takes an array of scancodes and outputs the results to an array of keysyms.
// The function returns both the lowercase and uppercase versions of the keysym,
// so `out_keysym` must be at least twice the length of `in_keycode`.
// The output goes as {lower0, upper0, lower1, upper1, ...}
// The `count` arguments are the amount of elements in their respective arrays.
bool get_xmodmap_keysym(
    const uint32_t *restrict in_scancode, size_t in_count,
    uint32_t *restrict out_keysym, size_t out_count
);

// Encode an Unicode value into an UTF-8 byte sequence (at most 4 bytes in total)
// Function returns the amount of bytes written to `*output`,
// with `output_size` being the buffer size.
// (a return value of zero means that the input is not a valid code point)
size_t codepoint_to_utf8(uint32_t codepoint, CharBuffer* output);

// Get the sequence of bytes emitted when the given scancodes are sent, using the uinput module
// This is an alternative strategy for getting the characters associated to scancodes,
// to be used in case xmodmap fails. However the program needs elevated privileges in order to use uinput.
// The circumnstances that this function is needed are expected to be quite rare,
// and I (the author) do not want to ask people to run the program with sudo.
// So I am letting this as a semi-hidden undocumented feature:
// in case the system does not have xmodmap, just run the game with sudo to get the character keys mapped.
bool get_uinput_chars(
    const uint32_t *restrict in_scancode, size_t in_count,
    CharBuffer *restrict out_chars, uint8_t *restrict out_char_size, size_t out_count
);

// Take an array of scancode values and output an array of the corresponding UTF-8 characters (lowercase and uppercase)
// Each of the arrays for storing the characters  and their sizes must have at least
// twice the amount of elements than the scancode array. An size of zero for the
// output character means that conversion failed for the corresponding scancode.
// Function returns `true` if all pointers are not NULL and the sizes are big enough, otherwise returns `false`.
bool scancodes_to_utf8(
    const uint32_t *restrict in_scancode,   // Array of scancode values
    size_t in_count,                        // Amount of elements in the scancode array
    CharBuffer *restrict out_char,          // Array to store the characters encoded in UTF-8
    uint8_t *restrict out_char_size,        // Array to store the sizes in bytes of each encoded character
    size_t out_count                        // Amount of elements in each of the two output arrays
);

#endif // _WIN32
