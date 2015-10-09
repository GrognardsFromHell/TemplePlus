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
	int32_t(__cdecl* encodeTriplet)(int32_t a1, int32_t a2, int32_t a3);




	void(__cdecl *UpdatePartyUI)();
	uint32_t(__cdecl *PartyMoney)();
	void(__cdecl *DebitPartyMoney)(int32_t platPcs, int32_t goldPcs, int32_t  silverPcs, int32_t copperPcs);


#pragma region Object Get / Set General
	int(*Obj_Get_Field_32bit)(objHndl objHnd, uint32_t nFieldIdx);
	objHndl(__cdecl *Obj_Get_Field_ObjHnd__fastout)(objHndl, _fieldIdx);
	uint64_t(__cdecl *Obj_Get_Field_64bit)(objHndl, _fieldIdx);
	int(__cdecl *Obj_Get_Field_Float)(objHndl, _fieldIdx);
	int(__cdecl *Obj_Get_IdxField_NumItems)(objHndl, _fieldIdx);
	uint32_t(__cdecl *Obj_Get_IdxField_32bit)(objHndl, _fieldIdx, _fieldSubIdx);
	uint64_t(__cdecl *Obj_Get_IdxField_64bit)(objHndl, _fieldIdx, _fieldSubIdx);
	objHndl(__cdecl *Obj_Get_IdxField_ObjHnd)(objHndl, _fieldIdx, _fieldSubIdx);
	int(__cdecl *Obj_Get_ArrayElem_Generic)(objHndl, _fieldIdx, _fieldSubIdx, void *pDataOut);
	int(__cdecl *Obj_Set_Field_32bit)(objHndl ObjHnd, uint32_t nFieldIdx, uint32_t n32Data);
	int(__cdecl *Obj_Set_Field_64bit)(objHndl, uint32_t nFieldIdx, uint64_t);
	int(__cdecl *Obj_Set_Field_ObjHnd)(objHndl, uint32_t nFieldIdx, objHndl);
	int(__cdecl *Obj_Set_IdxField_byValue)(objHndl, _fieldIdx, _fieldSubIdx, ...);
	void(__cdecl *Obj_Set_IdxField_byPtr)(objHndl, _fieldIdx, _fieldSubIdx, void * _SourceData);
	int(__cdecl *Obj_Set_IdxField_ObjHnd)(objHndl, _fieldIdx, _fieldSubIdx, objHndl);
#pragma endregion

	PyObject*  (__cdecl *PyObjFromObjHnd) (objHndl);
	uint64_t(__cdecl *GetProtoHandle)(uint32_t protoId);


	
	int32_t(__cdecl *ObjStatBaseGet)(objHndl, uint32_t obj_stat); // can return single precision floating point too (e.g. movement speed)
	int32_t(__cdecl *ObjStatBaseDispatch)(objHndl, uint32_t obj_stat, void *); // Dispatcher Type 0x42; defaults to ObjStatBaseGet if no Dispatcher found

	uint32_t (__cdecl *ObjGetBABAfterLevelling)(objHndl objHnd, Stat classBeingLevelledUp);
	uint32_t(__cdecl *XPReqForLevel)(uint32_t level);
	uint32_t(__cdecl *ObjXPGainProcess)(objHndl, uint32_t nXPGainRaw);

	uint32_t(__cdecl *sub_10152280)(objHndl, objHndl);	

	void (__cdecl *TurnProcessing)(objHndl obj);


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