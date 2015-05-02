
#pragma once

#include "util/addresses.h"

/*
	AAS seems to be the animation system.
*/
struct AasConfig {
	float pixelPerWorldTile1;
	float pixelPerWorldTile2;
	uint32_t getSkaFile; // Callback
	uint32_t getSkmFile; // Callback
	uint32_t getAnimName; // Callback, but default is used
	uint32_t anim_callback2; // Callback, but default is used
	uint32_t field18;
	void (__cdecl *runScript)(const char *command); // Python run script, probably for embedded animation scripts
};

struct AasFuncs : AddressTable {
	int (__cdecl *Init)(const AasConfig *config);

	AasFuncs() {
		rebase(Init, 0x102640B0);
	}
};
extern AasFuncs aasFuncs;
