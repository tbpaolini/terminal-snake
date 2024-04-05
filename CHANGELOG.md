**Version 1.0.5** - *April 5th, 2024*:
- In addition to the arrow keys, the character keys can also be used for movement (`WASD` keys in QWERTY, `ZQSD` in AZERTY, or the equivalent in other keyboards).

-----

**Version 1.0.4** - *March 12nd, 2024*:
- Fixed a potential buffer overflow when displaying an error message on Windows.
- Changed the input stream to unbuffered, so keyboard input can be parsed faster and so the stream can be cleared more reliably.
- Handling `Ctrl-Break` (Windows) and segmentation fault (Windows and Linux), so the terminal also gets restored to its original state in case the program terminates on such circunstances.

-----

**Version 1.0.3** - *February 29th, 2024*:
- Input parsing no longer breaks if the `char` type is unsiged on a C implementation.
- Compilation should no longer fail now if the function `__fpurge()` is not available on the target Linux system.
- Moved the check of if the terminal is big enough to right after its size is obtained (previously it was made a little later, which could result in dereferencing a `NULL` pointer).
- Check for zero size when allocating memory.
- Check if any dimension is zero when allocating a 2D array.
- On Linux, the pseudo-random number generator now seeds itself when it runs for the first time (on Windows, that already happened).

-----

**Version 1.0.2** - *February 27th, 2024*:
- Fixed bug that caused the snake to not collide with walls.

-----

**Version 1.0.1** - *February 27th, 2024*:
- Fixed bug in which the snake's head and tail being next to each other would register as a collision.
- Now the snake's speed is recalculated only when it changes, rather than on every frame.
- The game over screen now waits for 0.75 seconds before taking any input to exit the game.
- Corrected oversight that could make the final snake's to not be printed when exiting after the game over.
- Tweaks on the source code in order to ensure that it compiles with MSVC.

-----

**Version 1.0.0** - *February 23rd, 2024*:
- Initial release.
