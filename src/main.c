/* Note: Just compiling this 'main.c' file is enough for building the entire project. */

#include "includes.h"
#include "helper_functions.c"
#include "game_loop.c"

int main(int argc, char **argv)
{
    game_init();
    getchar();
    
    return 0;
}
