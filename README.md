# Snake game on the terminal

![terminal-snake](https://github.com/tbpaolini/terminal-snake/assets/85261542/b312b540-9e59-4bd7-9299-490ac5d13d2a)

* [Download the executable for Windows](https://github.com/tbpaolini/terminal-snake/releases/download/v1.0.5/snake.exe) (206 KB)
* [Download the executable for Linux](https://github.com/tbpaolini/terminal-snake/releases/download/v1.0.5/snake) (948 KB)

## About

This is the classic Snake game from old Nokia phones, but running inside the terminal. The snake is always moving and the goal is to guide it to the food pellets on the board. The snake grows and speeds up as it eat more pellets. It is game over if the snake hits an wall or itself. The snake can change direction with the arrow keys, and it moves faster if you press the same direction that the snake is facing. The game can be paused with the `ESC` key. Try getting as many pellets as possible!

In addition to the arrow keys, the snake can also be moved with the `WASD` keys (QWERTY keyboard), `ZQSD` (AZERTY keyboards), or the equivalent in other keyboards. This key mapping is independent of the keyboard layout, the physical position of the movement keys are the same regardless of which characters they represent.

You can change the game speed by passing a value from 1 to 12 as an argument to this program. The greater the value, the faster the snake moves. For example:
```shell
./snake 9
```
The default speed is 5 if you run the game without any arguments.

The size of the game area is determined by the size of the terminal. So if you want to play with a bigger or smaller area, you can do it by changing the size of your terminal window before starting the game.

## How to compile

This project makes use of an unity build, which means that you only need to compile the `main.c` file (inside the `src` directory) and then the entire project will be built. No need to set-up some build system. In theory, any compiler that supports the C11 standard (or later) should work. We recommend enabling speed optimizations and statically linking the libraries, since they do not seem to cause any bugs or increase the executable's size too much.

For reference, on Linux this project may be compiled with either GCC or Clang:
```shell
gcc src/main.c -o snake -O3 -static
```
```shell
clang src/main.c -o snake -O3 -static
```

On Windows, it is necessary to link the compiled program with the Microsoft's Universal C Runtime (UCRT) and with `user32.lib`. It is also necessary for UTF-8 support to be enabled, otherwise special characters might not be displayed properly. If you are using the MSVC compiler (from Visual Studio), this should work:
```shell
cl src/main.c /O2 /MT /GL /utf-8 /Fe: snake.exe /link user32.lib
```

Alternatively, you can use on Windows version of GCC or Clang that come on MingW. Just be sure of using their UCRT version, which support UTF-8 out-of-the-box. In such case, either of these should work:
```shell
gcc src/main.c -o snake.exe -O3 -static
```
```shell
clang src/main.c -o snake.exe -O3 -static
```

It is worth noting that you are not limited to using those compilers and flags. :-)

### Current compilation status

[![Building the project](https://github.com/tbpaolini/terminal-snake/actions/workflows/build.yml/badge.svg)](https://github.com/tbpaolini/terminal-snake/actions/workflows/build.yml)
[![Security analysis](https://github.com/tbpaolini/terminal-snake/actions/workflows/codeql.yml/badge.svg)](https://github.com/tbpaolini/terminal-snake/actions/workflows/codeql.yml)

## Technical details

This game is basically implemented using escape sequences, which allow to specify the colors and position for each character on the terminal. Linux typically supports escape sequences out-of-the-box. Windows also supports, but the program needs to enable them first by setting the console flag `ENABLE_VIRTUAL_TERMINAL_PROCESSING`. Actually, Microsoft recommends using escape sequences over their regular Win32 API when manipulating the terminal.

The snake is drawn by precisely controlling where and when the special characters are drawn, and the terminal screen is only updated once per frame. Only the parts of the screen that changed are updated. In order to help with that, a double-ended queue is used for storing the coordinates for all snake's parts: at the beginning of each step the head's coordinate is added the front of the queue, while the tail's coordinate is removed from the back. A 2D array is used as a collision grid, in order to determine if the snake's head got into the same space as another body part or an wall.

On the frames in which the snake gets a pellet, we skip the step in which the snake's tail is popped from the queue, this way the snake grows by one unit. After that, a new pellet randomly spawn on a space inside the game area where there is no part of snake. For this, it is generated a random number between zero and the amount of free spaces minus one, then the free spaces are looped over until the counter of free spaces exceeds the generated value, and the new pellet is placed there. Each empty space has an equal probability of being chosen, this program uses the pseudo-random number generator from the operating system, instead of the standard `rand()`. The generator is seeded with bytes from the entropy source of the OS, instead of seeding with the time.

The snake's speed is tied the update rate of the terminal screen, since the snake is moved every time the screen is updated. The time between updates is controlled during runtime with the precision of microseconds. That is accomplished by sleeping the program until a few milliseconds before the target time, then repeatedly checking if the target time was reached. As the snake gets more pellets, this time gradually decreases, which makes the snake to move faster. Pressing the same direction as the snake halves the time, and the speed value set when launching the game applies a modifier to the time.

The pressed keys are parsed from the input stream. Each arrow key emits a specific escape sequence there, which is read by the game. In order to also allow for movement with the character keys, on startup the game checks which characters are mapped to the scan codes `{17, 30, 31, 32}` (equivalent to `WASD` in QWERTY). Then the game also checks for those characters when parsing the input. Though the positions of the characters might change depending of the keyboard layout, the scan code that a key emit remain the same for the same physical position of the key. This way the movement keys can be mapped in a consistent way across different keyboards.

All in all, on each frame the game performs this loop:
1. Get the user's input.
2. Change the snake's direction or speed accordingly.
3. Update the snake's position and speed, then draw it.
4. Check if a collision happened.
5. If so, end the game. Otherwise, wait until enough time has passed since the last screen update.
