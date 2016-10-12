
#include "infrastructure/elfhash.h"

uint32_t ElfHash::Hash(const char * text)
{
	uint32_t hash = 0, g;

	if (text == nullptr)
	{
		return 0;
	}
		

	while (*text) {
		auto ch = *text++;

		// ToEE uses upper case elf hashes
		if (ch >= 'a' && ch <= 'z') {
			ch -= 32;
		}

		hash = (hash << 4) + ch;
		g = hash & 0xF0000000L;
		if (g) {
			hash ^= g >> 24;
		}
		hash &= ~g;
	}
	return hash;
}


