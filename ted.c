#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <SDL2/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"
#include "./v2.h"
#include "./editor.h"

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

#define BACKGROUND_COLOR 0x3c3c3cff

// Global variables (at the moment...)
Editor editor = {0};
Line line = {0};
size_t cursor = 0;
float zoom_factor = 1.0;

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

    return sdl_check_pointer(SDL_CreateRGBSurfaceFrom((void*)pixels, width, height, depth, pitch, rmask, gmask, bmask, amask));
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

void set_texture_color(SDL_Texture *texture, Uint32 color)
{
    sdl_check_code(SDL_SetTextureColorMod(texture, UNPACK_RGB(color)));
    sdl_check_code(SDL_SetTextureAlphaMod(texture, UNPACK_ALPHA(color)));
}

void render_char(SDL_Renderer *renderer, Font *font, const char c, Vec2f pos)
{
    const SDL_Rect dst = {
        .x = (int) floor(pos.x),
        .y = (int) floor(pos.y),
        .w = FONT_CHAR_WIDTH * FONT_SCALE,
        .h = FONT_CHAR_HEIGHT * FONT_SCALE
    };

    // assert(index <= ASCII_TABLE_SIZE);
    const uint8_t index = c % ASCII_TABLE_SIZE;
    sdl_check_code(SDL_RenderCopy(renderer, font->spritesheet, &font->glyph_table[index], &dst));
}

void render_text_sized(SDL_Renderer *renderer, Font *font, const char *buffer, size_t buffer_size, Vec2f pos, Uint32 color)
{
    set_texture_color(font->spritesheet, color);

    for (size_t i = 0; i < buffer_size; ++i) {
        render_char(renderer, font, buffer[i], pos);
        pos.x += (float) (FONT_CHAR_WIDTH * FONT_SCALE);
    }
}

void render_cursor(SDL_Renderer *renderer, Font *font, Uint32 color)
{
    const Vec2f pos =
        vec2f(
        (float) editor.cursor_col * FONT_CHAR_WIDTH * FONT_SCALE,
        (float) editor.cursor_row * FONT_CHAR_WIDTH * FONT_SCALE
        );

    SDL_Rect rect = {
        .x = (int) floorf(pos.x),
        .y = (int )floorf(pos.y),
        .w = FONT_CHAR_WIDTH * FONT_SCALE,
        .h = FONT_CHAR_HEIGHT * FONT_SCALE
    };
    sdl_check_code(SDL_SetRenderDrawColor(renderer, UNPACK_RGBA(color)));
    sdl_check_code(SDL_RenderFillRect(renderer, &rect));

    const char *c = editor_char_under_cursor(&editor);
    if (c) {
        set_texture_color(font->spritesheet, BACKGROUND_COLOR);
        render_char(renderer, font, *c, pos);
    }

}

// @TODO: Blinking cursor (23-07-2022)
// @TODO: Multiple lines
// @TODO: Save/Load file
// @TODO: Support for extended ASCII (2^8) (04-08-2022)
// Read this related post: https://stackoverflow.com/a/41198513/553803

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
    bool lctrl = false;
    bool quit = false;

    // Start with some string
    char* title = "ted v0.1";
    editor_insert_text_before_cursor(&editor, title);
    editor_insert_new_line(&editor);

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
                        zoom_factor += 0.25;
                    break;
                }
                case SDLK_MINUS: {
                    if (lctrl) {
                        if (zoom_factor >= 0.75)
                            zoom_factor -= 0.25;
                    } break;
                }
                case SDLK_RETURN: {
                    editor_insert_new_line(&editor);
                    break;
                }
                case SDLK_BACKSPACE: {
                    editor_backspace(&editor);
                    break;
                }
                case SDLK_DELETE: {
                    editor_delete(&editor);
                    break;
                }
                case SDLK_UP: {
                    if (editor.cursor_row > 0)
                        editor.cursor_row -= 1;
                    break;
                }
                case SDLK_DOWN: {
                    editor.cursor_row += 1;
                    break;
                }
                case SDLK_LEFT: {
                    if (editor.cursor_col > 0) {
                        editor.cursor_col -= 1;
                    }
                    break;
                }
                case SDLK_RIGHT: {
                    editor.cursor_col += 1;
                    break;
                }
                case SDLK_LCTRL: {
                    lctrl = true;
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
                editor_insert_text_before_cursor(&editor, event.text.text);
            }
        }
        // Render background color
        sdl_check_code(SDL_SetRenderDrawColor(renderer, UNPACK_RGBA(BACKGROUND_COLOR)));
        sdl_check_code(SDL_RenderClear(renderer));

        // render multiple lines
        for (size_t row = 0; row < editor.size; ++row) {
            Line *line = editor.lines + row;
            render_text_sized(renderer, &font, line->chars, line->size,
                              vec2f(0, row * FONT_CHAR_HEIGHT * FONT_SCALE * zoom_factor),
                              0xffffffff);
        }
        // and then... render the cursor
        render_cursor(renderer, &font, 0xffffffff);

        SDL_RenderPresent(renderer);
        SDL_Delay(30);
    }

    SDL_DestroyTexture(font.spritesheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return(0);
}
