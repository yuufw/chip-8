#include "roms_embedded.h"
#include "roms_manifest.h"

const rom_t *
roms_embedded_list(size_t *count)
{
#if ROMS_EMBEDDED_COUNT > 0
	static const rom_t roms[] = { ROMS_EMBEDDED_LIST };
#else
	static const rom_t *roms = NULL;
#endif

	if (count) {
		*count = ROMS_EMBEDDED_COUNT;
	}

	return roms;
}
