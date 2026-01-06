#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#define START_ADDR   0x200U
#define MEM_SIZE     4096U
#define FONTSET_ADDR ((uint16_t)(0x50))

typedef struct __attribute__((packed)) {
	uint8_t memory[MEM_SIZE];
} Memory;

void memory_init(Memory *m);
int memory_read(const Memory *m, uint16_t addr);
int memory_write(Memory *m, uint16_t addr, uint8_t val);
int memory_load_rom(Memory *m, const char *path);
int memory_load_buffer(Memory *m, const uint8_t *data, size_t size);

extern const uint8_t chip8_fontset[80];

#endif    // MEMORY_H
