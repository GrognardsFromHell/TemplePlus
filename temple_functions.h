#pragma once

#include "stdafx.h"
#include "tig/tig_font.h"
#include "addresses.h"
#include "dependencies/python-2.2/Python.h"
#include "common.h"

// Contains the function definitions for stuff found in temple.dll that we may want to call or override.

extern "C"
{
	int __declspec(dllimport) __cdecl temple_main(HINSTANCE hInstance, HINSTANCE hPrevInstance, const char* lpCommandLine, int nCmdShow);
}



struct locationSec {
	uint64_t location;

	int x() const {
		return location & 0x3FFFFFF;
	}
	int y() const {
		return (location >> 26) & 0x3FFFFFF;
	}
};



struct TempleFuncs : AddressTable {
	void(*ProcessSystemEvents)();
	PyObject* (__cdecl *PyScript_Execute)(char *pPyFileName, char *pPyFuncName, PyObject *pPyArgTuple);
	uint32_t(__cdecl *StringHash)( char * pString);


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
	int(__cdecl *Obj_Get_IdxField_256bit)(objHndl, _fieldIdx, _fieldSubIdx, void *);
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

	bool(__cdecl *StandPointPacketGet)(_jmpPntID jmpPntID, char * mapNameOut, size_t, _mapNum * mapNumOut, locXY * locXYOut);
	uint32_t(__cdecl *ObjStandpointGet)(objHndl, _standPointType, StandPoint *);
	uint32_t(__cdecl *ObjStandpointSet)(objHndl, _standPointType, StandPoint *);

	uint32_t (__cdecl *ObjGetBABAfterLevelling)(objHndl objHnd, Stat classBeingLevelledUp);
	uint32_t(__cdecl *XPReqForLevel)(uint32_t level);
	uint32_t(__cdecl *ObjXPGainProcess)(objHndl, uint32_t nXPGainRaw);

	uint32_t(__cdecl *sub_10152280)(objHndl, objHndl);

	

	//PyObject* (*PyObjFromObjHnd)();



	uint32_t(__cdecl *ObjSpellKnownQueryGetData)(objHndl objHnd, uint32_t spellEnums, uint32_t *_classCodes, uint32_t *_spellLevels, uint32_t *_numSpellsFound);
	uint32_t(__cdecl *ObjGetMaxSpellSlotLevel)(objHndl ObjHnd, uint32_t statClassIdx, uint32_t arg3);


	

	void (__cdecl *TurnProcessing)(objHndl obj);

	/*
		Generates a random integer using the configured random number generator.
	*/
	int (__cdecl *RandomIntRange)(int from, int to);


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

	int (*temple_snprintf)(char *, size_t, const char *, ...);

	uint32_t(__cdecl *sub_100664B0)(objHndl objHnd, uint32_t);

	// rebase on init
	TempleFuncs() {
		rebase(ProcessSystemEvents, 0x101DF440);
		rebase(PyScript_Execute, 0x100ADE40);
		rebase(StringHash, 0x101EBB00);

		rebase(UpdatePartyUI, 0x10134CB0);
		rebase(PartyMoney, 0x1002B750);
		rebase(DebitPartyMoney, 0x1002C020);



# pragma region Obj Get/Set General

		rebase(Obj_Get_Field_32bit, 0x1009E1D0);
		rebase(Obj_Get_Field_64bit, 0x1009E2E0);
		rebase(Obj_Get_Field_Float, 0x1009E260);
		rebase(Obj_Get_Field_ObjHnd__fastout, 0x1009E360);


		rebase(Obj_Get_IdxField_NumItems, 0x1009E7E0);

		rebase(Obj_Get_IdxField_32bit, 0x1009E5C0);
		rebase(Obj_Get_IdxField_64bit, 0x1009E640);
		rebase(Obj_Get_IdxField_ObjHnd, 0x1009E6D0);
		rebase(Obj_Get_IdxField_256bit, 0x1009E770);

		rebase(Obj_Set_Field_32bit, 0x100A0190);
		rebase(Obj_Set_Field_64bit, 0x100A0200);
		rebase(Obj_Set_Field_ObjHnd, 0x100A0280);
		rebase(Obj_Set_IdxField_byValue, 0x100A1310);
		rebase(Obj_Set_IdxField_byPtr, 0x100A1540);
		rebase(Obj_Set_IdxField_ObjHnd, 0x100A14A0);


#pragma endregion

		rebase(PyObjFromObjHnd, 0x100AF1D0);
		rebase(GetProtoHandle, 0x1003AD70);


		rebase(ObjStatBaseGet, 0x10074CF0);
		rebase(ObjStatBaseDispatch, 0x1004E810);

		rebase(ObjGetBABAfterLevelling, 0x100749B0);
		rebase(XPReqForLevel, 0x100802E0);
		rebase(ObjXPGainProcess, 0x100B5480);

		rebase(sub_10152280, 0x10152280);
		rebase(CraftMagicArmsAndArmorSthg, 0x10150B20);




		rebase(StandPointPacketGet, 0x100BDE20);
		rebase(ObjStandpointGet, 0x100BA890);
		rebase(ObjStandpointSet, 0x100BA8F0);


		rebase(ObjSpellKnownQueryGetData, 0x100762D0);
		rebase(ObjGetMaxSpellSlotLevel, 0x100765B0);



		rebase(_ItemWorthFromEnhancements, 0x101509C0);
		rebase(ItemCreationPrereqSthg_sub_101525B0, 0x101525B0);

		rebase(TurnProcessing, 0x100634E0);
		rebase(RandomIntRange, 0x10038DF0);

		rebase(temple_snprintf, 0x10254680);

		rebase(sub_100664B0, 0x100664B0);

	}
private:

	// usercall... eax has field id, ecx has type


	// usercall... eax has sthg to do with Magic Arms and Armor crafting
	bool(__cdecl *_ItemWorthFromEnhancements)();
};

enum Skills {
	skill_appraise = 0,
	skill_bluff,
	skill_concentration,
	skill_diplomacy,
	skill_disable_device,
	skill_gather_information,
	skill_heal,
	skill_hide,
	skill_intimidate,
	skill_listen,
	skill_move_silently,
	skill_open_lock,
	skill_pick_pocket,
	skill_search,
	skill_sense_motive,
	skill_spellcraft,
	skill_spot,
	skill_use_magic_device,
	skill_tumble,
	skill_wilderness_lore,
	skill_perform,
	skill_alchemy,
	skill_balance,
	skill_climb,
	skill_craft,
	skill_decipher_script,
	skill_disguise,
	skill_escape_artist,
	skill_forgery,
	skill_handle_animal,
	skill_innuendo,
	skill_intuit_direction,
	skill_jump,
	skill_knowledge_arcana,
	skill_knowledge_religion,
	skill_knowledge_nature,
	skill_knowledge_all,
	skill_profession,
	skill_read_lips,
	skill_ride,
	skill_swim,
	skill_use_rope
};

extern TempleFuncs templeFuncs;


const int CONTAINER_MAX_ITEMS = 1000; // may need to be reduced
const int CRITTER_MAX_ITEMS = 24; // definitely needs to be increased in the future :)
const int CRITTER_EQUIPPED_ITEM_SLOTS = 16;
const int CRITTER_EQUIPPED_ITEM_OFFSET = 200;

void __cdecl hooked_print_debug_message(char* format, ...);

void init_functions();
void init_hooks();
