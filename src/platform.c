#include "platform.h"
#include "chip8.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

static SDL_Window *win = NULL;
static SDL_Renderer *ren = NULL;
static TTF_Font *font = NULL;

int
platform_init_font(void)
{
#ifdef USE_SDL
	if (TTF_Init() != 0) {
		fprintf(stderr, "[ERROR] TTF_Init failed: %s\n",
		        TTF_GetError());
		return -ENOMEM;
	}

	font = TTF_OpenFont("assets/FreeMono.ttf", 18);
	if (!font) {
		fprintf(stderr, "[ERROR] Failed to load font: %s\n",
		        TTF_GetError());
		return -ENOMEM;
	}
#endif
	return 0;
}

typedef struct {
    SDL_Keycode sdl;
    uint8_t chip8;
} KeyMap;

static const KeyMap keymap[] = {
    { SDLK_1, 0x1 }, { SDLK_2, 0x2 }, { SDLK_3, 0x3 }, { SDLK_4, 0xC },
    { SDLK_q, 0x4 }, { SDLK_w, 0x5 }, { SDLK_e, 0x6 }, { SDLK_r, 0xD },
    { SDLK_a, 0x7 }, { SDLK_s, 0x8 }, { SDLK_d, 0x9 }, { SDLK_f, 0xE },
    { SDLK_z, 0xA }, { SDLK_x, 0x0 }, { SDLK_c, 0xB }, { SDLK_v, 0xF },
};

static int
map_key(SDL_Keycode sym)
{
    for (size_t i = 0; i < sizeof(keymap) / sizeof(keymap[0]); i++) {
        if (keymap[i].sdl == sym) {
            return keymap[i].chip8;
        }
    }
    return -1;
}

void
platform_handle_input(Input *in)
{
#ifdef USE_SDL
	if (!in) {
		return;
	}

	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_QUIT:
			in->exit_signal = 1;
			break;

		case SDL_KEYDOWN:
		case SDL_KEYUP: {
			bool down = (ev.type == SDL_KEYDOWN);
			int key = map_key(ev.key.keysym.sym);

			if (key >= 0) {
				input_set_key(in, (uint8_t)key, down);
			} else if (ev.key.keysym.sym == SDLK_ESCAPE) {
				in->exit_signal = 1;
			}
		} break;

		default:
			break;
		}
	}
#endif
}

SDL_Renderer *
platform_get_renderer(void)
{
	return ren;
}

void
platform_update(const Chip8 *c8)
{
#ifdef USE_SDL
	if (!c8) {
		return;
	}

	SDL_Color on = ORANGE_VINTAGE_CRT;
	SDL_Color off = CRT_DARK_BG;

	for (size_t y = 0; y < DISPLAY_HEIGHT; y++) {
		for (size_t x = 0; x < DISPLAY_WIDTH; x++) {
			uint8_t pixel = c8->disp.pixels[y * DISPLAY_WIDTH + x];

			SDL_Color col = pixel ? on : off;

			SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, 255);

			SDL_Rect rect = { (int)(x * SCREEN_SCALE),
				          (int)(y * SCREEN_SCALE), SCREEN_SCALE,
				          SCREEN_SCALE };

			SDL_RenderFillRect(ren, &rect);
		}
	}

	SDL_RenderPresent(ren);
#endif
}

int
platform_create_window(const char *title, int x, int y, int w, int h,
                       SDL_Window **out_win, SDL_Renderer **out_ren)
{
#ifdef USE_SDL
	if (!title || !out_win || !out_ren) {
		return -EINVAL;
	}

	*out_win = SDL_CreateWindow(title, x, y, w, h, SDL_WINDOW_SHOWN);
	if (!*out_win) {
		return -ENOMEM;
	}
	*out_ren = SDL_CreateRenderer(*out_win, -1, SDL_RENDERER_ACCELERATED);
	if (!*out_ren) {
		return -ENOMEM;
	}
#endif

	return 0;
}

int
platform_render_text(SDL_Renderer *renderer, int x, int y, const char *text)
{
#ifdef USE_SDL
	if (!renderer || !text) {
		return -EINVAL;
	}

	SDL_Color fg = ORANGE_VINTAGE_CRT;

	SDL_Surface *surf = TTF_RenderText_Blended(font, text, fg);
	if (!surf) {
		return -ENOMEM;
	}

	SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);
	if (!tex) {
		return -ENOMEM;
	}

	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

	SDL_Rect dst = { x, y, 0, 0 };
	SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	SDL_RenderCopy(renderer, tex, NULL, &dst);
	SDL_DestroyTexture(tex);
#endif
	return 0;
}

int
platform_render_text_col(SDL_Renderer *renderer, int x, int y, const char *text,
                         SDL_Color color)
{
#ifdef USE_SDL
	if (!renderer || !text) {
		return -EINVAL;
	}

	SDL_Surface *surf = TTF_RenderText_Blended(font, text, color);
	if (!surf) {
		return -ENOMEM;
	}

	SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);
	if (!tex) {
		return -ENOMEM;
	}

	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

	SDL_Rect dst = { x, y, 0, 0 };
	SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	SDL_RenderCopy(renderer, tex, NULL, &dst);
	SDL_DestroyTexture(tex);
#endif
	return 0;
}

int
platform_init(const char *title, int x, int y, int w, int h)
{
#ifdef USE_SDL
	if (!title) {
		return -EINVAL;
	}
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
		return -ENOMEM;
	}
	platform_init_font();
	return platform_create_window(title, x, y, w, h, &win, &ren);
#endif
	return 0;
}

void
platform_shutdown(void)
{
#ifdef USE_SDL
	if (ren) {
		SDL_DestroyRenderer(ren);
	}
	if (win) {
		SDL_DestroyWindow(win);
	}
	if (font) {
		TTF_CloseFont(font);
	}
	TTF_Quit();
	SDL_Quit();
#endif
}
