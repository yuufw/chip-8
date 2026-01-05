#include "chip8.h"
#include "debugger.h"
#include "platform.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: %s <ROM file>\n", argv[0]);
		return 1;
	}

	Chip8 c8;
	chip8_init(&c8);

	if (memory_load_rom(&c8.mem, argv[1]) != 0) {
		fprintf(stderr, "Failed to load ROM: %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	if (platform_init("Chip8 Emulator", SDL_WINDOWPOS_CENTERED,
	                  SDL_WINDOWPOS_CENTERED, DISPLAY_WIDTH_SCALED,
	                  DISPLAY_HEIGHT_SCALED) != 0) {
		fprintf(stderr, "Failed to initialize SDL platform\n");
		return 1;
	}

	if (platform_init_debugger("Chip8 Instructions", DEBUGGER_DISPLAY_X,
	                           DEBUGGER_DISPLAY_Y, DEBUGGER_DISPLAY_WIDTH,
	                           DEBUGGER_DISPLAY_HEIGHT) != 0) {
		fprintf(stderr, "Failed to initialize SDL debug platform\n");
		return 1;
	}
	uint32_t last_cpu_tick = SDL_GetTicks();
	uint32_t last_timer_tick = SDL_GetTicks();

	while (!c8.input.exit_signal) {
		uint32_t now = SDL_GetTicks();
		platform_handle_input(&c8.input);

		if (c8.input.waiting_for_key >= 0) {
			for (uint8_t k = 0; k < NUM_KEYS; k++) {
				if (c8.input.keys[k]) {
					c8.V[c8.input.waiting_for_key] = k;
					c8.input.waiting_for_key = -1;
					break;
				}
			}
		}

		if (now - last_cpu_tick >= CYCLE_DELAY_MS) {
			chip8_cycle(&c8);
			platform_update(&c8);
			debugger_render_registers(&c8);
			last_cpu_tick = now;
		}

		if (now - last_timer_tick >= TIMER_MS) {
			timers_update(&c8.timers);
			last_timer_tick = now;
		}
	}
	platform_shutdown();
	debugger_shutdown();
	return 0;
}
