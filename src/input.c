#include "input.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if defined(__x86_64__) || defined(_M_X64)
#define ARCH_ALIGN 8
#elif defined(__i386__) || defined(_M_IX86)
#define ARCH_ALIGN 4
#else
#define ARCH_ALIGN sizeof(void *)
#endif

static inline void
prefetch_memory(const void *ptr, size_t len)
{
	uintptr_t addr = (uintptr_t)ptr;
	uintptr_t end = addr + len;

	for (; addr < end; addr += ARCH_ALIGN) {
		volatile uint8_t tmp = *(uint8_t *)addr;
		(void)tmp;
	}
}

void
input_init(Input *in)
{
	prefetch_memory(in, sizeof(*in));
	memset(in->keys, 0, sizeof in->keys);
	in->waiting_for_key = -1;
	in->exit_signal = 0;
}

void
input_set_key(Input *in, uint8_t key, uint8_t pressed)
{
	if (key < NUM_KEYS) {
		in->keys[key] = pressed;
	}
}

void
input_wait_for_key(Input *in, uint8_t register_idx)
{
	in->waiting_for_key = register_idx;
}
