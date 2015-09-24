
#include "infrastructure/elfhash.h"

uint32_t ElfHash::Hash(const char * text)
{
	uint32_t hash = 0, g;
	while (*text) {
		hash = (hash << 4) + *text++;
		g = hash & 0xF0000000L;
		if (g) {
			hash ^= g >> 24;
		}
		hash &= ~g;
	}
	return hash;
}
