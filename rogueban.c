#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

#define FONT "./perfect_dos_font/perfect_dos_vga_437_regular_24.PNG"

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

SDL_Surface *get_suface_from_file(const char *file_path)
{
    // Source code robbed from https://wiki.libsdl.org/SDL_CreateRGBSurfaceFrom
    int req_format = STBI_rgb_alpha;
    int width, height, orig_format;
    unsigned char* pixels = stbi_load(file_path, &width, &height, &orig_format, req_format);
    if(pixels == NULL) {
        SDL_Log("Loading image failed: %s", stbi_failure_reason());
        exit(1);
    }

    // Set up the pixel format color masks for RGB(A) byte arrays.
    // Only STBI_rgb (3) and STBI_rgb_alpha (4) are supported here!
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int shift = (req_format == STBI_rgb) ? 8 : 0;
    const Uint32 rmask = 0xff000000 >> shift;
    const Uint32 gmask = 0x00ff0000 >> shift;
    const Uint32 bmask = 0x0000ff00 >> shift;
    const Uint32 amask = 0x000000ff >> shift;
#else // little endian, like x86
    const Uint32 rmask = 0x000000ff;
    const Uint32 gmask = 0x0000ff00;
    const Uint32 bmask = 0x00ff0000;
    const Uint32 amask = (req_format == STBI_rgb) ? 0 : 0xff000000;
#endif

    // STBI_rgb_alpha (RGBA)
    int depth = 32;
    int pitch = 4*width;

    return sdl_check_pointer(SDL_CreateRGBSurfaceFrom((void*)pixels, width, height, depth, pitch,
                                                      rmask, gmask, bmask, amask));
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv[0];
    sdl_check_code(SDL_Init(SDL_INIT_VIDEO));
    SDL_Window *window = sdl_check_pointer(SDL_CreateWindow("Rogueban", 0, 0, 512, 512, SDL_WINDOW_RESIZABLE));
    SDL_Renderer *renderer = sdl_check_pointer(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));
    SDL_Surface *font_surface = sdl_check_pointer( get_suface_from_file(FONT));
    SDL_Texture *font_texture = sdl_check_pointer(SDL_CreateTextureFromSurface(renderer, font_surface));
    SDL_Rect font_rect = (SDL_Rect) {
        .x = 0,
        .y = 0,
        .w = 18,
        .h = 25
    };

    SDL_Rect output_rect = (SDL_Rect) {
        .x = 0,
        .y = 0,
        .w = 18 * 2,
        .h = 25 * 2
    };

    size_t count = 0;

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
        }

        sdl_check_code(SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255));
        sdl_check_code(SDL_RenderClear(renderer));

        count++;
        if (count % 10 == 0)
            output_rect.x += 1;
        if (output_rect.x >= 512 + font_rect.w)
            output_rect.x = -font_rect.w;
        sdl_check_code(SDL_RenderCopy(renderer, font_texture, &font_rect, &output_rect));
        SDL_RenderPresent(renderer);
    }

    SDL_Quit();
    return(0);
}