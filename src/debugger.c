#include "debugger.h"

#include <errno.h>

static SDL_Window *debugger_reg_window = NULL;
static SDL_Renderer *debugger_reg_renderer = NULL;

typedef enum {
	REG_V0 = 0,
	REG_VF = 15,
	REG_I = 16,
	REG_PC = 17,
	REG_SP = 18,
	REG_DT = 19,
	REG_COUNT = 20
} DebuggerRegister;

int
debugger_render_registers(const Chip8 *c8)
{
#ifdef USE_SDL
	if (!debugger_reg_renderer || !c8) {
		return -EINVAL;
	}

	SDL_SetRenderDrawColor(debugger_reg_renderer, CRT_DARK_BG.r,
	                       CRT_DARK_BG.g, CRT_DARK_BG.b, CRT_DARK_BG.a);

	SDL_RenderClear(debugger_reg_renderer);

	char text_buffer[64];

	for (DebuggerRegister reg = 0; reg < REG_COUNT; reg++) {
		size_t grid_col = reg / DEBUGGER_GRID_WIDTH;
		size_t grid_row = reg % DEBUGGER_GRID_WIDTH;

		switch (reg) {
		case REG_V0 ... REG_VF:
			snprintf(text_buffer, sizeof(text_buffer),
			         "V%lX = %02X", (unsigned long)reg, c8->V[reg]);
			break;

		case REG_I:
			snprintf(text_buffer, sizeof(text_buffer), "I  = %03X",
			         c8->I);
			break;

		case REG_PC:
			snprintf(text_buffer, sizeof(text_buffer), "PC = %03X",
			         c8->pc);
			break;

		case REG_SP:
			snprintf(text_buffer, sizeof(text_buffer), "SP = %X",
			         c8->sp);
			break;

		case REG_DT:
			snprintf(text_buffer, sizeof(text_buffer), "DT = %02X",
			         c8->timers.dt);
			break;
		default:
			break;
		}

		platform_render_text(
			debugger_reg_renderer,
			DEBUGGER_TEXT_OFFSET_X + (int)(grid_col * 110),
			DEBUGGER_TEXT_OFFSET_Y + (int)(grid_row * 20),
			text_buffer);
	}

	SDL_RenderPresent(debugger_reg_renderer);
#endif
	return 0;
}

int
platform_init_debugger(const char *title, int x, int y, int w, int h)
{
#ifdef USE_SDL
	if (!title) {
		return -EINVAL;
	}

	return platform_create_window(title, x, y, w, h, &debugger_reg_window,
	                              &debugger_reg_renderer);
#endif
	return 0;
}

void
debugger_shutdown()
{
#ifdef USE_SDL
	if (debugger_reg_renderer) {
		SDL_DestroyRenderer(debugger_reg_renderer);
		debugger_reg_renderer = NULL;
	}
	if (debugger_reg_window) {
		SDL_DestroyWindow(debugger_reg_window);
		debugger_reg_window = NULL;
	}
#endif
}
