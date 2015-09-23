
#pragma once

#include <string>

class ElfHash {
public:

	static uint32_t Hash(const std::string &text) {
		return Hash(text.c_str());
	}

	static uint32_t Hash(const char *text);

};
