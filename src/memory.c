#include "memory.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

void
memory_init(Memory *m)
{
	memset(m->memory, 0, sizeof(m->memory));
	memcpy(&m->memory[FONTSET_ADDR], chip8_fontset, sizeof(chip8_fontset));
}

uint8_t
memory_read(const Memory *m, uint16_t addr)
{
	if (addr >= MEM_SIZE) {
		fprintf(stderr,
		        "Error: memory read out of bounds at address 0x%04X\n",
		        addr);
		return -EINVAL;
	}
	return m->memory[addr];
}

void
memory_write(Memory *m, uint16_t addr, uint8_t val)
{
	if (addr >= MEM_SIZE) {
		fprintf(stderr,
		        "Error: memory write out of bounds at address 0x%04X\n",
		        addr);
		return -EINVAL;
	}
	m->memory[addr] = val;
}

int
memory_load_rom(Memory *m, const char *path)
{
	FILE *file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "Error: could not open ROM file '%s'\n", path);
		return -ENOENT;
	}

	if (fseek(file, 0, SEEK_END) != 0) {
		fprintf(stderr,
		        "Error: could not seek to end of ROM file '%s'\n",
		        path);
		fclose(file);
		return -EIO;
	}

	long size = ftell(file);
	if (size < 1) {
		fprintf(stderr, "Error: could not get size of ROM file '%s'\n",
		        path);
		fclose(file);
		return -EINVAL;
	}

	if (fseek(file, 0, SEEK_SET) != 0) {
		fprintf(stderr,
		        "Error: could not seek to start of ROM file '%s'\n",
		        path);
		fclose(file);
		return -EIO;
	}

	if (START_ADDR + size > MEM_SIZE) {
		fprintf(stderr,
		        "Error: ROM file '%s' is too large to fit in memory\n",
		        path);
		fclose(file);
		return -ENOMEM;
	}

	size_t read = fread(&m->memory[START_ADDR], 1, (size_t)size, file);
	fclose(file);

	if (read != (size_t)size) {
		fprintf(stderr, "Error: could not read ROM file '%s'\n", path);
		return -EIO;
	}

	return 0;
}
