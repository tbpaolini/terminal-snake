/* Note: Just compiling this 'main.c' file is enough for building the entire project.

    For example, on Linux when using GCC or Clang any or these should work:
        gcc main.c -O3 -static -o snake
        clang main.c -O3 -static -o snake
   
    When compiling on Windows, this program should be linked with the Universal C Runtime (UCRT),
    and have UTF-8 support enabled. This command should accomplish those things with MSVC:
        cl main.c /O2 /MT /GL /utf-8 /Fe: snake.exe /link user32.lib

    The GCC and Clang for Windows that come on MingW also work, just be sure to be using the UCRT version.
    With those compilers on Windows, it is pretty much the same as compiling on Linux:
        gcc main.c -O3 -static -o snake.exe
        clang main.c -O3 -static -o snake.exe
*/

#include "includes.h"
#include "helper_functions.c"
#include "game_loop.c"
#include "game_logic.c"
#include "key_mapper.c"

int main(int argc, char **argv)
{
    unsigned int speed = parse_speed(argc, argv);
    GameState* state = game_init(speed);
    game_mainloop(state);
    game_close(state);
    
    return 0;
}
