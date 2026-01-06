#include "menu.h"

#include "colors.h"
#include "display.h"
#include "platform.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef USE_SDL

#define MENU_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

typedef enum Menu {
	MENU_TOP = 20u,
	MENU_TITLE_GAP = 30u,
	MENU_HELP_GAP = 30u,
	MENU_LINE_HEIGHT = 20u,
	MENU_LIST_GAP = 10u,
	MENU_INPUT_LABEL_GAP = 20u,
	MENU_INPUT_BOX_HEIGHT = 20u,
	MENU_HIGHLIGHT_WIDTH = 320u
} menu_t;

typedef struct MenuContext {
	const rom_t *roms;
	MenuSelection *out;
	MenuSelection *sel;
	const struct MenuKeyMap *menu_keys;
	const struct MenuKeyMap *input_keys;
	char *path;
	size_t *selected;
	size_t *path_len;
	int *input_mode;
	size_t rom_count;
	size_t menu_keys_len;
	size_t input_keys_len;
} menu_ctx_t;

typedef int (*MenuKeyHandler)(menu_ctx_t *ctx);

typedef struct MenuKeyMap {
	SDL_Keycode key;
	MenuKeyHandler handler;
} MenuKeyMap;

typedef int (*MenuEventHandler)(menu_ctx_t *ctx, const SDL_Event *ev);

typedef struct MenuEventMap {
	Uint32 type;
	MenuEventHandler handler;
} MenuEventMap;

static int menu_key_up(menu_ctx_t *ctx);
static int menu_key_down(menu_ctx_t *ctx);
static int menu_key_select(menu_ctx_t *ctx);
static int menu_key_open_input(menu_ctx_t *ctx);
static int menu_key_exit(menu_ctx_t *ctx);
static int menu_key_backspace(menu_ctx_t *ctx);
static int menu_key_accept_path(menu_ctx_t *ctx);
static int menu_key_cancel_input(menu_ctx_t *ctx);
static int menu_event_quit(menu_ctx_t *ctx, const SDL_Event *ev);
static int menu_event_textinput(menu_ctx_t *ctx, const SDL_Event *ev);
static int menu_event_keydown(menu_ctx_t *ctx, const SDL_Event *ev);

static const MenuKeyMap menu_keys[] = {
	[0] = { .key = SDLK_UP, .handler = menu_key_up },
	[1] = { .key = SDLK_DOWN, .handler = menu_key_down },
	[2] = { .key = SDLK_RETURN, .handler = menu_key_select },
	[3] = { .key = SDLK_KP_ENTER, .handler = menu_key_select },
	[4] = { .key = SDLK_l, .handler = menu_key_open_input },
	[5] = { .key = SDLK_ESCAPE, .handler = menu_key_exit }
};

static const MenuKeyMap input_keys[] = {
	[0] = { .key = SDLK_BACKSPACE, .handler = menu_key_backspace },
	[1] = { .key = SDLK_RETURN, .handler = menu_key_accept_path },
	[2] = { .key = SDLK_KP_ENTER, .handler = menu_key_accept_path },
	[3] = { .key = SDLK_ESCAPE, .handler = menu_key_cancel_input }
};

static const MenuEventMap menu_events[] = {
	[0] = { .type = SDL_QUIT, .handler = menu_event_quit },
	[1] = { .type = SDL_TEXTINPUT, .handler = menu_event_textinput },
	[2] = { .type = SDL_KEYDOWN, .handler = menu_event_keydown }
};

static int
menu_key_up(menu_ctx_t *ctx)
{
	if (ctx->rom_count > 0) {
		*ctx->selected = (*ctx->selected == 0) ? (ctx->rom_count - 1) :
		                                         (*ctx->selected - 1);
	}
	return 0;
}

static int
menu_key_down(menu_ctx_t *ctx)
{
	if (ctx->rom_count > 0) {
		*ctx->selected = (*ctx->selected + 1) % ctx->rom_count;
	}
	return 0;
}

static int
menu_key_select(menu_ctx_t *ctx)
{
	if (ctx->rom_count > 0) {
		ctx->sel->type = MENU_SELECTION_EMBEDDED;
		ctx->sel->index = *ctx->selected;
		*ctx->out = *ctx->sel;
		return 1;
	}
	return 0;
}

static int
menu_key_open_input(menu_ctx_t *ctx)
{
	*ctx->input_mode = 1;
	SDL_StartTextInput();
	return 0;
}

static int
menu_key_exit(menu_ctx_t *ctx)
{
	ctx->sel->type = MENU_SELECTION_EXIT;
	*ctx->out = *ctx->sel;
	return 1;
}

static int
menu_key_backspace(menu_ctx_t *ctx)
{
	if (*ctx->path_len > 0) {
		ctx->path[--(*ctx->path_len)] = '\0';
	}
	return 0;
}

static int
menu_key_accept_path(menu_ctx_t *ctx)
{
	if (*ctx->path_len > 0) {
		ctx->sel->type = MENU_SELECTION_PATH;
		memcpy(ctx->sel->path, ctx->path, *ctx->path_len + 1);
		*ctx->out = *ctx->sel;
		SDL_StopTextInput();
		return 1;
	}
	return 0;
}

static int
menu_key_cancel_input(menu_ctx_t *ctx)
{
	*ctx->input_mode = 0;
	SDL_StopTextInput();
	return 0;
}

static int
menu_dispatch_key(const MenuKeyMap *map, size_t map_len, SDL_Keycode key,
                  menu_ctx_t *ctx)
{
	for (size_t i = 0; i < map_len; i++) {
		if (map[i].key == key) {
			return map[i].handler(ctx);
		}
	}
	return 0;
}

static int
menu_event_quit(menu_ctx_t *ctx, const SDL_Event *ev)
{
	(void)ev;
	ctx->sel->type = MENU_SELECTION_EXIT;
	*ctx->out = *ctx->sel;
	SDL_StopTextInput();
	return 1;
}

static int
menu_event_textinput(menu_ctx_t *ctx, const SDL_Event *ev)
{
	if (*ctx->input_mode) {
		size_t add_len = strlen(ev->text.text);
		if (*ctx->path_len + add_len < ROM_PATH_MAX - 1) {
			memcpy(&ctx->path[*ctx->path_len], ev->text.text,
			       add_len);
			*ctx->path_len += add_len;
			ctx->path[*ctx->path_len] = '\0';
		}
	}
	return 0;
}

static int
menu_event_keydown(menu_ctx_t *ctx, const SDL_Event *ev)
{
	if (*ctx->input_mode) {
		return menu_dispatch_key(ctx->input_keys, ctx->input_keys_len,
		                         ev->key.keysym.sym, ctx);
	}

	return menu_dispatch_key(ctx->menu_keys, ctx->menu_keys_len,
	                         ev->key.keysym.sym, ctx);
}

static int
menu_dispatch_event(const MenuEventMap *map, size_t map_len,
                    const SDL_Event *ev, menu_ctx_t *ctx)
{
	for (size_t i = 0; i < map_len; i++) {
		if (map[i].type == ev->type) {
			return map[i].handler(ctx, ev);
		}
	}
	return 0;
}

static size_t
menu_items_per_page(SDL_Renderer *ren)
{
	int w = 0;
	int h = 0;
	int list_start = MENU_TOP + MENU_TITLE_GAP + MENU_HELP_GAP;
	int footer = MENU_LIST_GAP + MENU_INPUT_LABEL_GAP + MENU_LINE_HEIGHT;
	int available = 0;
	int items = 1;

	if (SDL_GetRendererOutputSize(ren, &w, &h) != 0 || h <= 0) {
		h = (int)DISPLAY_HEIGHT_SCALED;
	}

	available = h - list_start - footer;
	if (available > 0) {
		items = available / MENU_LINE_HEIGHT;
	}

	if (items < 1) {
		items = 1;
	}

	return (size_t)items;
}

static void
menu_display_name(const char *name, char *out, size_t out_len)
{
	size_t len = 0;
	const char *dot = NULL;

	if (!out || out_len == 0) {
		return;
	}

	if (!name) {
		out[0] = '\0';
		return;
	}

	dot = strrchr(name, '.');
	if (dot && dot != name) {
		len = (size_t)(dot - name);
	} else {
		len = strlen(name);
	}

	if (len >= out_len) {
		len = out_len - 1;
	}

	memcpy(out, name, len);
	out[len] = '\0';
}

static void
menu_render(SDL_Renderer *ren, const rom_t *roms, size_t rom_count,
            size_t selected, size_t top, const char *path, int input_mode)
{
	SDL_Color fg = ORANGE_VINTAGE_CRT;
	int y = MENU_TOP;
	size_t items_per_page = menu_items_per_page(ren);
	size_t end = top + items_per_page;
	if (end > rom_count) {
		end = rom_count;
	}

	SDL_SetRenderDrawColor(ren, CRT_DARK_BG.r, CRT_DARK_BG.g, CRT_DARK_BG.b,
	                       CRT_DARK_BG.a);
	SDL_RenderClear(ren);

	platform_render_text(ren, 20, y, "CHIP-8 ROM SELECT");
	y += MENU_TITLE_GAP;
	platform_render_text(
		ren, 20, y,
		"Up/Down: Select  Enter: Load  L: Load from path  Esc: Quit");
	y += MENU_HELP_GAP;

	if (rom_count == 0) {
		platform_render_text(ren, 20, y, "No embedded ROMs found.");
		y += MENU_LINE_HEIGHT;
	} else {
		for (size_t i = top; i < end; i++) {
			char line[128];
			char name[96];
			SDL_Color line_color = fg;

			if (!input_mode && i == selected) {
				SDL_Rect hi = { 16, y - 2, MENU_HIGHLIGHT_WIDTH,
					        MENU_LINE_HEIGHT - 2 };
				SDL_SetRenderDrawColor(ren, fg.r, fg.g, fg.b,
				                       40);
				SDL_RenderFillRect(ren, &hi);
				line_color = CRT_DARK_BG;
			}

			menu_display_name(roms[i].name, name, sizeof(name));
			snprintf(line, sizeof(line), "%s", name);
			platform_render_text_col(ren, 24, y, line, line_color);
			y += MENU_LINE_HEIGHT;
		}
	}

	y += MENU_LIST_GAP;
	platform_render_text(ren, 20, y, "Load from path:");
	y += MENU_INPUT_LABEL_GAP;

	if (input_mode) {
		SDL_Rect box = { 16, y - 2, 500, MENU_INPUT_BOX_HEIGHT };
		SDL_SetRenderDrawColor(ren, fg.r, fg.g, fg.b, 40);
		SDL_RenderFillRect(ren, &box);
	}

	platform_render_text_col(ren, 24, y,
	                         path && path[0] ? path : "(press L)",
	                         input_mode ? CRT_DARK_BG : fg);

	SDL_RenderPresent(ren);
}

int
menu_select_rom(const rom_t *roms, size_t rom_count, MenuSelection *out)
{
	if (!out) {
		return -EINVAL;
	}

	SDL_Renderer *ren = platform_get_renderer();
	if (!ren) {
		return -EINVAL;
	}

	MenuSelection sel = { 0 };
	size_t selected = 0;
	size_t top = 0;
	char path[ROM_PATH_MAX] = { 0 };
	size_t path_len = 0;
	int input_mode = 0;

	menu_ctx_t ctx = { .roms = roms,
		           .rom_count = rom_count,
		           .out = out,
		           .sel = &sel,
		           .selected = &selected,
		           .input_mode = &input_mode,
		           .path = path,
		           .path_len = &path_len,
		           .menu_keys = menu_keys,
		           .menu_keys_len = MENU_ARRAY_LEN(menu_keys),
		           .input_keys = input_keys,
		           .input_keys_len = MENU_ARRAY_LEN(input_keys) };

	for (;;) {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			if (menu_dispatch_event(menu_events,
			                        MENU_ARRAY_LEN(menu_events),
			                        &ev, &ctx)) {
				return 0;
			}
		}

		if (rom_count > 0) {
			size_t items_per_page = menu_items_per_page(ren);
			size_t max_top = 0;
			if (rom_count > items_per_page) {
				max_top = rom_count - items_per_page;
			}

			if (selected < top) {
				top = selected;
			} else if (selected >= top + items_per_page) {
				top = selected - items_per_page + 1;
			}

			if (top > max_top) {
				top = max_top;
			}
		} else {
			top = 0;
		}

		menu_render(ren, roms, rom_count, selected, top, path,
		            input_mode);
		SDL_Delay(16);
	}
}

#else

int
menu_select_rom(const rom_t *roms, size_t rom_count, MenuSelection *out)
{
	(void)roms;
	(void)rom_count;
	(void)out;
	return -ENOTSUP;
}

#endif
