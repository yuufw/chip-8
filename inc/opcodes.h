#ifndef CHIP8_OPS_H
#define CHIP8_OPS_H

#include "chip8.h"
#include <stdint.h>

typedef void (*Chip8Op)(Chip8 *c8, uint16_t opcode);

void dispatch(Chip8 *c8, uint16_t opcode);

#endif    // CHIP8_OPS_H
