#pragma once

#include <temple/dll.h>
#include "common.h"
#include "temple_enums.h"

// Contains the function definitions for stuff found in temple.dll that we may want to call or override.

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


	void(*ProcessSystemEvents)();
	uint32_t(__cdecl *StringHash)(const char * pString);


	int32_t diceRoll(uint32_t dieNum, uint32_t dieType, int32_t dieBonus);
	int32_t (__cdecl*RNG)(uint32_t num, uint32_t range);
	int32_t(__cdecl* dicePack)(int32_t a1, int32_t a2, int32_t a3);




	void(__cdecl *UpdatePartyUI)();
	uint32_t(__cdecl *PartyMoney)();
	void(__cdecl *DebitPartyMoney)(int32_t platPcs, int32_t goldPcs, int32_t  silverPcs, int32_t copperPcs);

	PyObject*  (__cdecl *PyObjFromObjHnd) (objHndl);
	uint64_t(__cdecl *GetProtoHandle)(uint32_t protoId);


	
	int32_t(__cdecl *ObjStatBaseGet)(objHndl, uint32_t obj_stat); // can return single precision floating point too (e.g. movement speed)
	int32_t(__cdecl *ObjStatBaseDispatch)(objHndl, uint32_t obj_stat, void *); // Dispatcher Type 0x42; defaults to ObjStatBaseGet if no Dispatcher found

	uint32_t (__cdecl *ObjGetBABAfterLevelling)(objHndl objHnd, Stat classBeingLevelledUp);
	uint32_t(__cdecl *XPReqForLevel)(uint32_t level);
	uint32_t(__cdecl *ObjXPGainProcess)(objHndl, uint32_t nXPGainRaw);

	uint32_t(__cdecl *sub_10152280)(objHndl, objHndl);	

	void (__cdecl *CombatAdvanceTurn)(objHndl obj);


	uint32_t ItemWorthFromEnhancements(uint32_t n) {
		uint32_t result;
		__asm {
			push esi;
			push ecx;
			mov ecx, this;
			mov esi, [ecx]._ItemWorthFromEnhancements;
			mov eax, n;
			call esi;
			pop ecx;
			pop esi;
			mov result, eax
		}
		return result ;
	}
	uint32_t(__cdecl *CraftMagicArmsAndArmorSthg)(uint32_t n);
	char *(__cdecl *ItemCreationPrereqSthg_sub_101525B0)(objHndl, objHndl);
	
	uint32_t(__cdecl *sub_100664B0)(objHndl objHnd, uint32_t);

	// rebase on init

private:


	// usercall... eax has sthg to do with Magic Arms and Armor crafting
	bool(__cdecl *_ItemWorthFromEnhancements)();
};


extern TempleFuncs templeFuncs;


const int CONTAINER_MAX_ITEMS = 1000; // may need to be reduced
const int CRITTER_MAX_ITEMS = 24; // definitely needs to be increased in the future :)
const int CRITTER_EQUIPPED_ITEM_SLOTS = 16;
const int CRITTER_EQUIPPED_ITEM_OFFSET = 200;

int32_t _diceRoll(uint32_t dieNum, uint32_t dieType, int32_t dieBonus);