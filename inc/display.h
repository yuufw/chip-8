#ifndef DISPLAY_H
#define DISPLAY_H

#include "memory.h"
#include <stddef.h>
#include <stdint.h>

#define SCREEN_SCALE          10U
#define DISPLAY_WIDTH         64U
#define DISPLAY_HEIGHT        32U
#define DISPLAY_WIDTH_SCALED  (DISPLAY_WIDTH * SCREEN_SCALE)
#define DISPLAY_HEIGHT_SCALED (DISPLAY_HEIGHT * SCREEN_SCALE)

#define DEBUGGER_DISPLAY_WIDTH  220U
#define DEBUGGER_DISPLAY_HEIGHT 220U
#define DEBUGGER_DISPLAY_X      350U
#define DEBUGGER_DISPLAY_Y      100U

#define DISPLAY_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT)

typedef struct __attribute__((packed)) {
	uint8_t pixels[DISPLAY_SIZE];
} Display;

void display_init(Display *disp);
uint8_t display_draw_sprite(Display *disp, Memory *m, uint16_t I, uint8_t x,
                            uint8_t y, uint8_t n);

#endif    // DISPLAY_H
