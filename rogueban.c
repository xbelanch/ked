#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
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
    SDL_Window *window = sdl_check_pointer(SDL_CreateWindow("Rogueban", 0, 0, 512, 512, SDL_WINDOW_RESIZABLE));
    SDL_Renderer *renderer = sdl_check_pointer(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

    bool quit = false;

    while (!quit) {
        SDL_Event event = {0};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                quit = true;
                break;
            }
            }

            sdl_check_code(SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255));
            sdl_check_code(SDL_RenderClear(renderer));
            SDL_RenderPresent(renderer);
        }
    }

    SDL_Quit();
    return(0);
}