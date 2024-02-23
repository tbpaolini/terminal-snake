# Snake game on the terminal

![terminal-snake](https://github.com/tbpaolini/terminal-snake/assets/85261542/b312b540-9e59-4bd7-9299-490ac5d13d2a)

* [Download the executable for Windows](https://github.com/tbpaolini/terminal-snake/releases/download/v1.0.0/snake.exe)
* [Download the executable for Linux](https://github.com/tbpaolini/terminal-snake/releases/download/v1.0.0/snake)

## About

This is the classic Snake game from old Nokia phones, but running inside the terminal. The snake is always moving and the goal is to guide it to the food pellets on the board. The snake grows and speeds up as it eat more pellets. It is game over if the snake hits an wall or itself. The snake can change direction with the arrow keys, and it moves faster if you press the same direction that the snake is facing. The game can be paused with the `ESC` key. Try getting as many pellets as possible!

You can change the game speed by passing a value from 1 to 12 as an argument to this program. The greater the value, the faster the snake moves. For example:
```shell
./snake 9
```
The default speed is 5 if you run the game without any arguments.

The size of the game area is determined by the size of the terminal. So if you want to play with a bigger or smaller area, you can do it by changing the size of your terminal window before starting the game.

## Compiling

This project makes use of an unity build, which means that you only need to compile the `main.c` file (inside the `src` directory) and then the entire project will be built. No need to set-up some build system. For example, if you are using GCC, this should work:
```shell
gcc src/main.c -o snake -O3
```
