#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv[0];
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL Cannot init\n");
    }

    fprintf(stdout, "Hello, rogueban!\n");

    atexit(SDL_Quit);

    return 0;
}