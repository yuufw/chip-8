#include "opcodes.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* util */
static inline void op_invalid(Chip8 *, uint16_t);

/* 0x00** */
static inline void op_0xxx(Chip8 *, uint16_t);
static inline void op_00e0(Chip8 *, uint16_t);
static inline void op_00ee(Chip8 *, uint16_t);

/* 1NNN / 2NNN */
static inline void op_1nnn(Chip8 *, uint16_t);
static inline void op_2nnn(Chip8 *, uint16_t);

/* 3XKK / 4XKK / 5XY0 */
static inline void op_3xkk(Chip8 *, uint16_t);
static inline void op_4xkk(Chip8 *, uint16_t);
static inline void op_5xy0(Chip8 *, uint16_t);

/* 6XKK / 7XKK */
static inline void op_6xkk(Chip8 *, uint16_t);
static inline void op_7xkk(Chip8 *, uint16_t);

/* 8XY* */
static inline void op_8xxx(Chip8 *, uint16_t);
static inline void op_8xy0(Chip8 *, uint16_t);
static inline void op_8xy1(Chip8 *, uint16_t);
static inline void op_8xy2(Chip8 *, uint16_t);
static inline void op_8xy3(Chip8 *, uint16_t);
static inline void op_8xy4(Chip8 *, uint16_t);
static inline void op_8xy5(Chip8 *, uint16_t);
static inline void op_8xy6(Chip8 *, uint16_t);
static inline void op_8xy7(Chip8 *, uint16_t);
static inline void op_8xye(Chip8 *, uint16_t);

/* 9XY0 / ANNN / BNNN / CXKK */
static inline void op_9xy0(Chip8 *, uint16_t);
static inline void op_annn(Chip8 *, uint16_t);
static inline void op_bnnn(Chip8 *, uint16_t);
static inline void op_cxkk(Chip8 *, uint16_t);

/* DXYN */
static inline void op_dxyn(Chip8 *, uint16_t);

/* EX** */
static inline void op_exxx(Chip8 *, uint16_t);
static inline void op_ex9e(Chip8 *, uint16_t);
static inline void op_exa1(Chip8 *, uint16_t);

/* FX** */
static inline void op_fxxx(Chip8 *, uint16_t);
static inline void op_fx07(Chip8 *, uint16_t);
static inline void op_fx0a(Chip8 *, uint16_t);
static inline void op_fx15(Chip8 *, uint16_t);
static inline void op_fx18(Chip8 *, uint16_t);
static inline void op_fx1e(Chip8 *, uint16_t);
static inline void op_fx29(Chip8 *, uint16_t);
static inline void op_fx33(Chip8 *, uint16_t);
static inline void op_fx55(Chip8 *, uint16_t);
static inline void op_fx65(Chip8 *, uint16_t);

static Chip8Op chip8_main_table[16] = {
	[0x0] = op_0xxx, [0x1] = op_1nnn, [0x2] = op_2nnn, [0x3] = op_3xkk,
	[0x4] = op_4xkk, [0x5] = op_5xy0, [0x6] = op_6xkk, [0x7] = op_7xkk,
	[0x8] = op_8xxx, [0x9] = op_9xy0, [0xA] = op_annn, [0xB] = op_bnnn,
	[0xC] = op_cxkk, [0xD] = op_dxyn, [0xE] = op_exxx, [0xF] = op_fxxx,
};

static Chip8Op table0[256] = {
	[0xE0] = op_00e0,
	[0xEE] = op_00ee,
};

static Chip8Op table8[16] = {
	[0x0] = op_8xy0, [0x1] = op_8xy1, [0x2] = op_8xy2,
	[0x3] = op_8xy3, [0x4] = op_8xy4, [0x5] = op_8xy5,
	[0x6] = op_8xy6, [0x7] = op_8xy7, [0xE] = op_8xye,
};

static Chip8Op tableE[256] = {
	[0x9E] = op_ex9e,
	[0xA1] = op_exa1,
};

static Chip8Op tableF[256] = {
	[0x07] = op_fx07, [0x0A] = op_fx0a, [0x15] = op_fx15,
	[0x18] = op_fx18, [0x1E] = op_fx1e, [0x29] = op_fx29,
	[0x33] = op_fx33, [0x55] = op_fx55, [0x65] = op_fx65,
};

/* ============================================================
                 UTIL
============================================================ */

static inline void
op_invalid(Chip8 *c8, uint16_t op)
{
	fprintf(stderr, "Invalid op %04X at PC=%04X\n", op, c8->pc - 2);
}

/* ============================================================
   0x00**
============================================================ */

// CLS
static inline void
op_00e0(Chip8 *c8, uint16_t op)
{
	memset(c8->disp.pixels, 0, sizeof c8->disp.pixels);
}

// RET
static inline void
op_00ee(Chip8 *c8, uint16_t op)
{
	if (c8->sp == 0) {
		return;
	}
	c8->pc = c8->stack[--c8->sp];
}

static inline void
op_0xxx(Chip8 *c8, uint16_t op)
{
	table0[op & 0xFF](c8, op);
}

/* ============================================================
   1NNN / 2NNN
============================================================ */

// JMP
static inline void
op_1nnn(Chip8 *c8, uint16_t op)
{
	c8->pc = op & 0xFFF;
}

// CALL
static inline void
op_2nnn(Chip8 *c8, uint16_t op)
{
	c8->stack[c8->sp++] = c8->pc;
	c8->pc = op & 0xFFF;
}

/* ============================================================
   3XKK / 4XKK / 5XY0
============================================================ */

// SE Vx, byte
static inline void
op_3xkk(Chip8 *c8, uint16_t op)
{
	uint8_t x = (op >> 8) & 0xF;
	if (c8->V[x] == (op & 0xFF)) {
		c8->pc += 2;
	}
}

// SNE Vx, byte
static inline void
op_4xkk(Chip8 *c8, uint16_t op)
{
	uint8_t x = (op >> 8) & 0xF;
	if (c8->V[x] != (op & 0xFF)) {
		c8->pc += 2;
	}
}

// SE Vx, Vy
static inline void
op_5xy0(Chip8 *c8, uint16_t op)
{
	uint8_t x = (op >> 8) & 0xF;
	uint8_t y = (op >> 4) & 0xF;
	if (c8->V[x] == c8->V[y]) {
		c8->pc += 2;
	}
}

/* ============================================================
   6XKK / 7XKK
============================================================ */

// LD Vx, byte
static inline void
op_6xkk(Chip8 *c8, uint16_t op)
{
	c8->V[(op >> 8) & 0xF] = op & 0xFF;
}

// ADD Vx, byte
static inline void
op_7xkk(Chip8 *c8, uint16_t op)
{
	uint8_t x = (op >> 8) & 0xF;
	c8->V[x] = (uint8_t)(c8->V[x] + (op & 0xFF));
}

/* ============================================================
   8XY*
============================================================ */

// LD Vx, Vy
static inline void
op_8xy0(Chip8 *c8, uint16_t op)
{
	c8->V[(op >> 8) & 0xF] = c8->V[(op >> 4) & 0xF];
}

// OR Vx, Vy
static inline void
op_8xy1(Chip8 *c8, uint16_t op)
{
	c8->V[(op >> 8) & 0xF] |= c8->V[(op >> 4) & 0xF];
}

// AND Vx, Vy
static inline void
op_8xy2(Chip8 *c8, uint16_t op)
{
	c8->V[(op >> 8) & 0xF] &= c8->V[(op >> 4) & 0xF];
}

// XOR Vx, Vy
static inline void
op_8xy3(Chip8 *c8, uint16_t op)
{
	c8->V[(op >> 8) & 0xF] ^= c8->V[(op >> 4) & 0xF];
}

// ADD Vx, Vy
static inline void
op_8xy4(Chip8 *c8, uint16_t op)
{
	uint8_t x = (op >> 8) & 0xF, y = (op >> 4) & 0xF;
	uint16_t s = c8->V[x] + c8->V[y];
	c8->V[0xF] = s > 0xFF;
	c8->V[x] = (uint8_t)s;
}

// SUB Vx, Vy
static inline void
op_8xy5(Chip8 *c8, uint16_t op)
{
	uint8_t x = (op >> 8) & 0xF, y = (op >> 4) & 0xF;
	c8->V[0xF] = c8->V[x] > c8->V[y];
	c8->V[x] -= c8->V[y];
}

// SHR Vx {, Vy}
static inline void
op_8xy6(Chip8 *c8, uint16_t op)
{
	uint8_t x = (op >> 8) & 0xF;
	c8->V[0xF] = c8->V[x] & 1;
	c8->V[x] >>= 1;
}

// SUBN Vx, Vy
static inline void
op_8xy7(Chip8 *c8, uint16_t op)
{
	uint8_t x = (op >> 8) & 0xF, y = (op >> 4) & 0xF;
	c8->V[0xF] = c8->V[y] > c8->V[x];
	c8->V[x] = c8->V[y] - c8->V[x];
}

// SHL Vx {, Vy}
static inline void
op_8xye(Chip8 *c8, uint16_t op)
{
	uint8_t x = (op >> 8) & 0xF;
	c8->V[0xF] = (c8->V[x] >> 7) & 1;
	c8->V[x] <<= 1;
}

static inline void
op_8xxx(Chip8 *c8, uint16_t op)
{
	table8[op & 0xF](c8, op);
}

/* ============================================================
   9XY0 / ANNN / BNNN / CXKK
============================================================ */

// SNE Vx, Vy
static inline void
op_9xy0(Chip8 *c8, uint16_t o)
{
	if (c8->V[(o >> 8) & 0xF] != c8->V[(o >> 4) & 0xF]) {
		c8->pc += 2;
	}
}

// LD I, addr
static inline void
op_annn(Chip8 *c8, uint16_t o)
{
	c8->I = o & 0xFFF;
}

// JMP V0, addr
static inline void
op_bnnn(Chip8 *c8, uint16_t o)
{
	c8->pc = (o & 0xFFF) + c8->V[0];
}

// RND Vx, byte
static inline void
op_cxkk(Chip8 *c8, uint16_t o)
{
	c8->V[(o >> 8) & 0xF] = (rand() & 0xFF) & (o & 0xFF);
}

/* ============================================================
   DXYN
============================================================ */

// DRW Vx, Vy, nibble
static inline void
op_dxyn(Chip8 *c8, uint16_t o)
{
	uint8_t x = c8->V[(o >> 8) & 0xF];
	uint8_t y = c8->V[(o >> 4) & 0xF];
	uint8_t n = o & 0xF;
	c8->V[0xF] = display_draw_sprite(&c8->disp, &c8->mem, c8->I, x, y, n);
}

/* ============================================================
   EX**
============================================================ */

// SKP Vx
static inline void
op_ex9e(Chip8 *c8, uint16_t o)
{
	if (c8->input.keys[c8->V[(o >> 8) & 0xF]]) {
		c8->pc += 2;
	}
}

// SKNP Vx
static inline void
op_exa1(Chip8 *c8, uint16_t o)
{
	if (!c8->input.keys[c8->V[(o >> 8) & 0xF]]) {
		c8->pc += 2;
	}
}

static inline void
op_exxx(Chip8 *c8, uint16_t o)
{
	tableE[o & 0xFF](c8, o);
}

/* ============================================================
   FX**
============================================================ */

// LD Vx, DT
static inline void
op_fx07(Chip8 *c8, uint16_t o)
{
	c8->V[(o >> 8) & 0xF] = c8->timers.dt;
}

// LD Vx, K
static inline void
op_fx0a(Chip8 *c8, uint16_t o)
{
	input_wait_for_key(&c8->input, (o >> 8) & 0xF);
}

// LD DT, Vx
static inline void
op_fx15(Chip8 *c8, uint16_t o)
{
	c8->timers.dt = c8->V[(o >> 8) & 0xF];
}

// LD ST, Vx
static inline void
op_fx18(Chip8 *c8, uint16_t o)
{
	c8->timers.st = c8->V[(o >> 8) & 0xF];
}

// ADD I, Vx
static inline void
op_fx1e(Chip8 *c8, uint16_t o)
{
	c8->I += c8->V[(o >> 8) & 0xF];
}

// LD F, Vx
static inline void
op_fx29(Chip8 *c8, uint16_t o)
{
	c8->I = FONTSET_ADDR + c8->V[(o >> 8) & 0xF] * 5;
}

// LD B, Vx
static inline void
op_fx33(Chip8 *c8, uint16_t o)
{
	uint8_t v = c8->V[(o >> 8) & 0xF];
	memory_write(&c8->mem, c8->I, v / 100);
	memory_write(&c8->mem, c8->I + 1, (v / 10) % 10);
	memory_write(&c8->mem, c8->I + 2, v % 10);
}

// LD [I], Vx
static inline void
op_fx55(Chip8 *c8, uint16_t o)
{
	for (uint8_t i = 0; i <= ((o >> 8) & 0xF); i++) {
		memory_write(&c8->mem, c8->I + i, c8->V[i]);
	}
}

// LD Vx, [I]
static inline void
op_fx65(Chip8 *c8, uint16_t o)
{
	for (uint8_t i = 0; i <= ((o >> 8) & 0xF); i++) {
		c8->V[i] = memory_read(&c8->mem, c8->I + i);
	}
}

static inline void
op_fxxx(Chip8 *c8, uint16_t o)
{
	tableF[o & 0xFF](c8, o);
}

void
dispatch(Chip8 *c8, uint16_t opcode)
{
	Chip8Op op = chip8_main_table[(opcode >> 12) & 0xF];
	if (op) {
		op(c8, opcode);
	} else {
		op_invalid(c8, opcode);
	}
}
