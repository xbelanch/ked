#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
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
#define FONT_SCALE 3.0
#define MAX_NUMBER_CHAR_WIDTH  24
#define MAX_NUMBER_CHAR_HEIGHT 24
#define WINDOW_WIDTH  (FONT_CHAR_WIDTH * MAX_NUMBER_CHAR_WIDTH * FONT_SCALE)
#define WINDOW_HEIGHT (FONT_CHAR_HEIGHT * MAX_NUMBER_CHAR_WIDTH * FONT_SCALE)
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

    for (size_t idx = 0; idx < ASCII_TABLE_SIZE; ++idx) {
        const size_t col = idx % FONT_COLS;
        const size_t row = idx / FONT_ROWS;
        font.glyph_table[idx]= (SDL_Rect) { .x = col * FONT_CHAR_WIDTH, .y = row * FONT_CHAR_HEIGHT, .w = FONT_CHAR_WIDTH, .h = FONT_CHAR_HEIGHT };
    }

    return (font);
}

void render_char(SDL_Renderer *renderer, Font *font, const char c, Vec2f pos, float scale)
{
    const SDL_Rect dst = {
        .x = (int) floor(pos.x),
        .y = (int) floor(pos.y),
        .w = FONT_CHAR_WIDTH * scale,
        .h = FONT_CHAR_HEIGHT * scale
    };

    const size_t index = c;
    assert(index <= ASCII_TABLE_SIZE);
    sdl_check_code(SDL_RenderCopy(renderer, font->spritesheet, &font->glyph_table[index], &dst));
}

void render_text_sized(SDL_Renderer *renderer, Font *font, const char *buffer, size_t buffer_size, Vec2f pos, Uint32 color, float scale)
{
    sdl_check_code(SDL_SetTextureColorMod(font->spritesheet, UNPACK_RGB(color)));
    sdl_check_code(SDL_SetTextureAlphaMod(font->spritesheet, UNPACK_ALPHA(color)));

    for (size_t i = 0; i < buffer_size; ++i) {
        render_char(renderer, font, buffer[i], pos, scale);
        pos.x += (float) (FONT_CHAR_WIDTH * scale);
    }
}

void render_text(SDL_Renderer *renderer, Font *font, const char *text, Vec2f pos, Uint32 color, float scale)
{
    render_text_sized(renderer, font, text, strlen(text), pos, color, scale);
}


#define BUFFER_CAPACITY 1024 // 1 kilobyte bro!
char buffer[BUFFER_CAPACITY];
size_t buffer_size = 0;

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv[0];

    sdl_check_code(SDL_Init(SDL_INIT_VIDEO));
    SDL_Window *window =
        sdl_check_pointer(SDL_CreateWindow("Rogueban", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE));

    SDL_Renderer *renderer =
        sdl_check_pointer(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

    Font font = font_load_from_file(FONT, renderer, 0x0);

    Vec2f cursor = vec2f(0.0, 0.0);
    bool lctrl = false;
    float font_scale = FONT_SCALE;

    bool quit = false;
    while (!quit) {
        SDL_Event event = {0};
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                case SDLK_LCTRL: {
                    lctrl = false;
                    break;
                }
                }
            } else if (event.type == SDL_KEYDOWN ){
                switch (event.key.keysym.sym) {
                case SDLK_PLUS: {
                    if (lctrl)
                        font_scale += 1.0;
                    break;
                }
                case SDLK_MINUS: {
                    if (lctrl)
                        font_scale -= 1.0;
                    break;
                }
                case SDLK_BACKSPACE: {
                    if (buffer_size > 0)
                        buffer_size -= 1;
                    break;
                }
                case SDLK_LCTRL: {
                    lctrl = true;
                    break;
                }
                case SDLK_RETURN: {
                    // TODO: Implement that
                    // cursor.x = 0;
                    // cursor.y += 16;
                    break;
                }
                case SDLK_ESCAPE: {
                    quit = true;
                    break;
                }
                default: break;
                }
            }
            else if (event.type == SDL_TEXTINPUT && !lctrl) {
                // Copy text input from keyboard to buffer
                size_t text_size = strlen(event.text.text);
                const size_t free_space = BUFFER_CAPACITY - buffer_size;
                if (text_size > free_space)
                    text_size = free_space;

                memcpy(buffer + buffer_size, event.text.text, text_size);
                buffer_size += text_size;
            }
        }

        sdl_check_code(SDL_SetRenderDrawColor(renderer, UNPACK_RGBA(0xaaaaaaff)));
        sdl_check_code(SDL_RenderClear(renderer));
        render_text_sized(renderer, &font, buffer, buffer_size, cursor, 0x0ff, font_scale);
        SDL_RenderPresent(renderer);
        SDL_Delay(30);
    }

    SDL_DestroyTexture(font.spritesheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return(0);
}