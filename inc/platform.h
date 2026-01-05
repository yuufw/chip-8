#ifndef PLATFORM_H
#define PLATFORM_H

#include "chip8.h"
#include "colors.h"
#include "input.h"
#include <SDL2/SDL.h>

int platform_init(const char *title, int x, int y, int w, int h);
int platform_init_font(void);
int platform_create_window(const char *title, int x, int y, int w, int h,
                           SDL_Window **out_win, SDL_Renderer **out_ren);
void platform_update(const Chip8 *c8);
int platform_render_text(SDL_Renderer *ren, int x, int y, const char *text);
void platform_handle_input(Input *in);
void platform_shutdown(void);
#endif    // PLATFORM_H
