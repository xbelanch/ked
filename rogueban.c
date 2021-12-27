#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

void sdl_check_code(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
        exit(1);
    }
}

void *sdl_check_pointer(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
        exit(1);
    }
    return ptr;
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv[0];
    sdl_check_code(SDL_Init(SDL_INIT_VIDEO));

    SDL_Quit();
    return(0);
}