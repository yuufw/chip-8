#include "display.h"

void
display_init(Display *disp)
{
	for (uint32_t i = 0; i < (uint32_t)(DISPLAY_WIDTH * DISPLAY_HEIGHT);
	     i++) {
		disp->pixels[i] = 0u;
	}
}

uint8_t
display_draw_sprite(Display *disp, Memory *m, uint16_t I, uint8_t x, uint8_t y,
                    uint8_t n)
{
	uint8_t collision = 0;

	for (uint8_t row = 0; row < n; row++) {
		uint8_t sprite_byte = m->memory[(uint32_t)I + row];

		for (uint8_t col = 0; col < 8u; col++) {
			if (sprite_byte & (uint8_t)(0x80u >> col)) {
				uint8_t px =
					(uint8_t)((x + col) % DISPLAY_WIDTH);
				uint8_t py =
					(uint8_t)((y + row) % DISPLAY_HEIGHT);
				uint32_t index =
					(uint32_t)py * DISPLAY_WIDTH + px;

				if (disp->pixels[index] == 1u) {
					collision = 1;
				}

				disp->pixels[index] ^= 1u;
			}
		}
	}

	return collision;
}
