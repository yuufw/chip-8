#ifndef ROMS_EMBEDDED_H
#define ROMS_EMBEDDED_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	const char *name;
	const char *section;
	const uint8_t *start;
	const uint8_t *end;
} rom_t;

const rom_t *roms_embedded_list(size_t *count);

static inline size_t
embedded_rom_size(const rom_t *rom)
{
	return (size_t)(rom->end - rom->start);
}

#endif    // ROMS_EMBEDDED_H
