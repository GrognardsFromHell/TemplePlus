#pragma once

#include <temple/dll.h>
#include "common.h"
#include "temple_enums.h"

// This is an old file from when we had only started with Temple+, containing a hodge podge of various functions. We'll get rid of it eventually :P

struct locationSec {
	uint64_t location;

	int x() const {
		return location & 0x3FFFFFF;
	}
	int y() const {
		return (location >> 26) & 0x3FFFFFF;
	}
};



struct TempleFuncs : temple::AddressTable {
	TempleFuncs();

	uint32_t(__cdecl *StringHash)(const char * pString);


	int32_t diceRoll(uint32_t dieNum, uint32_t dieType, int32_t dieBonus);
	int32_t (__cdecl*RNG)(uint32_t num, uint32_t range);


	void(__cdecl *UpdatePartyUI)();

	
	int32_t(__cdecl *ObjStatBaseDispatch)(objHndl, uint32_t obj_stat, void *); // Dispatcher Type 0x42; defaults to ObjStatBaseGet if no Dispatcher found

	
	uint32_t(__cdecl *sub_100664B0)(objHndl objHnd, uint32_t);

};


extern TempleFuncs templeFuncs;


const int CONTAINER_MAX_ITEMS = 1000; // may need to be reduced
const int CRITTER_MAX_ITEMS = 24; // definitely needs to be increased in the future :)
const int CRITTER_EQUIPPED_ITEM_SLOTS = 16;
const int CRITTER_EQUIPPED_ITEM_OFFSET = 200;

int32_t _diceRoll(uint32_t dieNum, uint32_t dieType, int32_t dieBonus);