#ifndef MENU_H
#define MENU_H

#include "roms_embedded.h"

#include <stddef.h>

#define ROM_PATH_MAX 512U

typedef enum {
	MENU_SELECTION_EXIT = 0,
	MENU_SELECTION_EMBEDDED,
	MENU_SELECTION_PATH
} MenuSelectionType;

typedef struct {
	MenuSelectionType type;
	size_t index;
	char path[ROM_PATH_MAX];
} MenuSelection;

int menu_select_rom(const rom_t *roms, size_t rom_count,
                    MenuSelection *out);

#endif    // MENU_H
