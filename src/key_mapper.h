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

// TO DO: Windows specific functions...

#else

// Convert a keysym to the code point of its character on the Unicode table
// If successful, return 'true' and write the value to '*codepoint'. Otherwise, return 'false'.
bool keysym_to_codepoint(uint32_t keysym, uint32_t* codepoint);

#endif // _WIN32