/* Rationale:
    The goal of this translation unit is to automatically map keys in an way that is independent from the keyboard layout.
    If we just mapped the "WASD" keys as the directional keys, that would only work in QWERTY keyboards (like US English).
    But if we map the key's scan codes instead, then the key's position should be the same regardless of the keyboard layout.
    For example, what would be "WASD" in QWERTY keyboards becomes "ZQSD" in AZERTY or ",AOE" in Dvorak.

    Note: The arrow keys can still be used in addition to the "WASD"-style keys mapped by this translation unit.
*/

#pragma once

#include "includes.h"

#ifdef _WIN32

// Sequence of two 16-bit values for storing a characer encoded in UTF-16
typedef uint16_t utf16_char_t[2];

// Take an array of scancode values and output an array of the corresponding UTF-16 characters (lowercase and uppercase)
// Each of the arrays for storing the characters  and their sizes must have at least
// twice the amount of elements than the scancode array. An size of zero for the
// output character means that conversion failed for the corresponding scancode.
// Function returns `true` if all pointers are not NULL and the sizes are big enough, otherwise returns `false`.
bool scancodes_to_utf16(
    const uint32_t *restrict in_scancode,   // Array of scancode values
    size_t in_count,                        // Amount of elements in the scancode array
    utf16_char_t *restrict out_char,        // Array to store the characters encoded in UTF-16
    uint8_t *restrict out_char_size,        // Array to store the sizes in bytes of each encoded character
    size_t out_count                        // Amount of elements in each of the two output arrays
);

#else

// Sequence of 4 bytes for stroring a character encoded in UTF-8
typedef uint8_t utf8_char_t[4];

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
size_t codepoint_to_utf8(uint32_t codepoint, utf8_char_t* output);

// Take an array of scancode values and output an array of the corresponding UTF-8 characters (lowercase and uppercase)
// Each of the arrays for storing the characters  and their sizes must have at least
// twice the amount of elements than the scancode array. An size of zero for the
// output character means that conversion failed for the corresponding scancode.
// Function returns `true` if all pointers are not NULL and the sizes are big enough, otherwise returns `false`.
bool scancodes_to_utf8(
    const uint32_t *restrict in_scancode,   // Array of scancode values
    size_t in_count,                        // Amount of elements in the scancode array
    utf8_char_t *restrict out_char,         // Array to store the characters encoded in UTF-8
    uint8_t *restrict out_char_size,        // Array to store the sizes in bytes of each encoded character
    size_t out_count                        // Amount of elements in each of the two output arrays
);

#endif // _WIN32
