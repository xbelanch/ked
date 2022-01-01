#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <SDL2/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"
#include "./v2.h"

#define FONT "./font/8x8.png"
#define FONT_COLS 16
#define FONT_ROWS 16
#define FONT_WIDTH 128
#define FONT_HEIGHT 128
#define FONT_CHAR_WIDTH  (FONT_WIDTH  / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)
#define SCALE 3
#define MAX_NUMBER_CHAR_WIDTH  24
#define MAX_NUMBER_CHAR_HEIGHT 24
#define WINDOW_WIDTH  (FONT_CHAR_WIDTH * MAX_NUMBER_CHAR_WIDTH * SCALE)
#define WINDOW_HEIGHT (FONT_CHAR_HEIGHT * MAX_NUMBER_CHAR_WIDTH * SCALE)
#define ASCII_TABLE_SIZE 128

#define UNPACK_RGBA(color)  (Uint8)(color>>24),(Uint8)(color>>16),(Uint8)(color>>8),(color&0xff)
#define UNPACK_RGB(color) (Uint8)(color>>24),(Uint8)(color>>16),(Uint8)(color>>8)
#define UNPACK_ALPHA(color) (color&0xff)

typedef struct {
    SDL_Texture *spritesheet;
    SDL_Rect glyph_table[ASCII_TABLE_SIZE];
} Font;

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

Font font_load_from_file(const char *filepath, SDL_Renderer *renderer, Uint32 colorKey)
{
    Font font = {0};
    SDL_Surface *font_surface = sdl_check_pointer(get_suface_from_file(filepath));
    SDL_SetColorKey(font_surface, SDL_TRUE, colorKey);
    font.spritesheet = sdl_check_pointer(SDL_CreateTextureFromSurface(renderer, font_surface));
    SDL_FreeSurface(font_surface);

    // TODO: Initialize glyph table

    return (font);
}

void render_char(SDL_Renderer *renderer, SDL_Texture *font, const char c, Vec2f pos, Uint32 color, float scale)
{
    const size_t index = c;
    const size_t col = index % FONT_COLS;
    const size_t row = index / FONT_ROWS;

    const SDL_Rect src = { .x = col * FONT_CHAR_WIDTH, .y = row * FONT_CHAR_HEIGHT, .w = FONT_CHAR_WIDTH, .h = FONT_CHAR_HEIGHT };
    const SDL_Rect dst = { .x = (int) floor(pos.x), .y = (int) floor(pos.y), .w = FONT_CHAR_WIDTH * scale, .h = FONT_CHAR_HEIGHT * scale };

    sdl_check_code(SDL_SetTextureColorMod(font, UNPACK_RGB(color)));
    sdl_check_code(SDL_SetTextureAlphaMod(font, UNPACK_ALPHA(color)));
    sdl_check_code(SDL_RenderCopy(renderer, font, &src, &dst));
}

void render_text(SDL_Renderer *renderer, SDL_Texture *font, const char *text, Vec2f pos, Uint32 color, float scale)
{
    size_t len = strlen(text);
    Vec2f cursor = pos;
    for (size_t i = 0; i < len; ++i) {
        render_char(renderer, font, text[i], cursor, color, scale);
        cursor.x += (float) (FONT_CHAR_WIDTH * scale);
    }
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv[0];

    sdl_check_code(SDL_Init(SDL_INIT_VIDEO));
    SDL_Window *window = sdl_check_pointer(SDL_CreateWindow("Rogueban", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE));
    SDL_Renderer *renderer = sdl_check_pointer(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));
    Font font = font_load_from_file(FONT, renderer, 0x0);
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

        sdl_check_code(SDL_SetRenderDrawColor(renderer, UNPACK_RGBA(0xaaaaaaff)));
        sdl_check_code(SDL_RenderClear(renderer));
        render_text(renderer, font.spritesheet, "10 PRINT \"HELLO, WORLD!@", vec2f(0.0, 0.0), 0xff0000ff, 3.0f);
        render_text(renderer, font.spritesheet, "20 GOTO 10", vec2f(0.0, 24.0), 0xff00ffff, 3.0f);
        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }

    SDL_DestroyTexture(font.spritesheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return(0);
}