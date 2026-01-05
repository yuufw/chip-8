#include "chip8.h"
#include "opcodes.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static inline uint16_t
chip8_fetch(const Chip8 *c8)
{
	const uint8_t hi = memory_read(&c8->mem, c8->pc);
	const uint8_t lo = memory_read(&c8->mem, c8->pc + 1);
	return (uint16_t)((hi << 8) | lo);
}

void
chip8_cycle(Chip8 *c8)
{
	const uint16_t opcode = chip8_fetch(c8);
	c8->pc += 2;
	dispatch(c8, opcode);
}

void
chip8_init(Chip8 *c8)
{
	memory_init(&c8->mem);
	display_init(&c8->disp);
	input_init(&c8->input);

	c8->I = 0;
	c8->pc = START_ADDR;
	c8->sp = 0;

	for (size_t i = 0; i < CHIP8_NUM_REGS; i++) {
		c8->V[i] = 0;
	}

	for (size_t i = 0; i < CHIP8_STACK_LEN; i++) {
		c8->stack[i] = 0;
	}
}
