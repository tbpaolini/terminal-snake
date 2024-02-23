/* Note: Just compiling this 'main.c' file is enough for building the entire project. */

#include "includes.h"
#include "helper_functions.c"
#include "game_loop.c"
#include "game_logic.c"

int main(int argc, char **argv)
{
    unsigned int speed = parse_speed(argc, argv);
    GameState* state = game_init(speed);
    game_mainloop(state);
    game_close(state);
    
    return 0;
}
