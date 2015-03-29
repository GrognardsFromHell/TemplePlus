#pragma once

#include "stdafx.h"
#include "addresses.h"
#include "dependencies/python-2.2/Python.h"
#include "temple_enums.h"
#include "spell.h"

// Contains the function definitions for stuff found in temple.dll that we may want to call or override.

extern "C"
{
	int __declspec(dllimport) __cdecl temple_main(HINSTANCE hInstance, HINSTANCE hPrevInstance, const char* lpCommandLine, int nCmdShow);
}

typedef uint64_t objHndl;
typedef uint32_t _fieldIdx;
typedef uint32_t _fieldSubIdx;
typedef uint32_t _mapNum;
typedef uint32_t _jmpPntID;
typedef uint32_t _standPointType;
typedef uint32_t _featCode;
typedef uint32_t _key;

# pragma region Standard Structs

struct ObjectId {
	uint16_t subtype;
	uint16_t something;
	uint32_t field4; // always zero from what I've seen
	GUID guid;
};

struct locXY{
	uint32_t locx;
	uint32_t locy;
};

struct Loc_And_Offsets{
	locXY location;
	float off_x;
	float off_y;
};

struct LocFull {
	Loc_And_Offsets location;
	float off_z;
};

struct GroupArray {
	objHndl GroupMembers[32];
	uint32_t * GroupSize;
	void* unknownfunc;
};


struct StandPoint {
	uint32_t mapNum;
	uint32_t field4;
	Loc_And_Offsets LocAndOff;
	_jmpPntID jmpPntID;
	uint32_t field_1C;
};

struct JumpPointPacket{
	_jmpPntID jmpPntID;
	char * pMapName;
	_mapNum mapNum;
	uint32_t field_C;
	locXY location;
};

#pragma endregion

struct TempleFuncs : AddressTable {
	void(*ProcessSystemEvents)();
	void(__cdecl *UpdatePartyUI)();
	uint32_t(__cdecl *PartyMoney)();
	void(__cdecl *DebitPartyMoney)(int32_t copperPcs, int32_t silverPcs, int32_t goldPcs, int32_t platPcs);


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

	int(__cdecl *IsObjDeadNullDestroyed)(objHndl);
	PyObject*  (__cdecl *PyObjFromObjHnd) (objHndl);
	const char *(__cdecl *ObjGetDisplayName)(uint64_t obj, uint64_t observer);

	int(__cdecl *DispatcherD20Query)(objHndl ObjHnd, _key nKey);

	// GroupArray stuff (groups include: NPCs, PCs, AI Followers, Currently Selected)
	objHndl(__cdecl *GroupArrayMemberN)(GroupArray *, int nIdx);
	objHndl(__cdecl *GroupNPCFollowersGetMemberN)(int nIdx);
	int(__cdecl *GroupNPCFollowersLen)();
	objHndl(__cdecl *GroupPCsGetMemberN)(int nIdx);
	int(__cdecl *GroupPCsLen)();
	objHndl(__cdecl *GroupListGetMemberN)(int nIdx);
	int(__cdecl *GroupListGetLen)();
	int(__cdecl *ObjIsInGroupArray)(GroupArray *, objHndl);
	int(__cdecl *ObjIsAIFollower)(objHndl);
	int(__cdecl * ObjFindInGroupArray)(GroupArray *, objHndl); // returns index
	int(__cdecl * ObjRemoveFromAllGroupArrays)(objHndl);
	int(__cdecl *ObjAddToGroupArray)(GroupArray *, objHndl);
	int(__cdecl *ObjAddToPCGroup)(objHndl);



	objHndl(__cdecl *ObjGetSubstituteInventory)  (objHndl);
	objHndl(__cdecl *ObjGetItemAtInventoryLocation)(objHndl, uint32_t nIdx);

	int(__cdecl *ObjStatGet)(objHndl, int obj_stat);

	bool(__cdecl *StandPointPacketGet)(_jmpPntID jmpPntID, char * mapNameOut, size_t, _mapNum * mapNumOut, locXY * locXYOut);
	int(__cdecl *ObjStandpointGet)(objHndl, _standPointType, StandPoint *);
	int(__cdecl *ObjStandpointSet)(objHndl, _standPointType, StandPoint *);

	int(__cdecl *ObjFactionHas)(objHndl, int nFaction);
	int(__cdecl *ObjPCHasFactionFromReputation)(objHndl, int nFaction);
	int(__cdecl *ObjFactionAdd)(objHndl, int nFaction);

	uint32_t(__cdecl *XPReqForLevel)(uint32_t level);
	int(__cdecl *ObjXPGainProcess)(objHndl, int nXPGainRaw);

	int(__cdecl *sub_10152280)(objHndl, objHndl);

	int(__cdecl *ObjFeatAdd)(objHndl, _featCode);

	//PyObject* (*PyObjFromObjHnd)();

	uint64_t(__cdecl *GetProtoHandle)(int protoId);

	int(__cdecl *ObjSpellKnownQueryGetData)(objHndl objHnd, uint32_t spellEnums, uint32_t *_classCodes, uint32_t *_spellLevels, uint32_t *_numSpellsFound);
	int(__cdecl *ObjGetMaxSpellSlotLevel)(objHndl ObjHnd, uint32_t statClassIdx, uint32_t arg3);


	bool DoesTypeSupportField(uint32_t objType, _fieldIdx objField) {
		int result;
		__asm {
			push esi;
			push ecx;
			mov ecx, this;
			mov esi, [ecx]._DoesObjectFieldExist;
			mov ecx, objType;
			mov eax, objField;
			call esi;
			pop ecx;
			pop esi;
			mov result, eax
		}
		return result != 0;
	}


	bool ItemWorthFromEnhancements(uint32_t n) {
		int result;
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
	uint32_t(__cdecl *CraftMagicArmsAndArmorSthg)(int n);

	// rebase on init
	TempleFuncs() {
		rebase(ProcessSystemEvents, 0x101DF440);
		rebase(UpdatePartyUI, 0x10134CB0);
		rebase(PartyMoney, 0x1002B750);
		rebase(DebitPartyMoney, 0x1002C020);

		rebase(GetProtoHandle, 0x1003AD70);

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

		rebase(DispatcherD20Query, 0x1004CC00);

		rebase(IsObjDeadNullDestroyed, 0x1007E650);
		rebase(PyObjFromObjHnd, 0x100AF1D0);
		rebase(ObjGetDisplayName, 0x1001FA80);


		rebase(GroupArrayMemberN, 0x100DF760);
		rebase(GroupNPCFollowersGetMemberN, 0x1002B190);
		rebase(GroupNPCFollowersLen, 0x1002B360);
		rebase(GroupPCsGetMemberN, 0x1002B170);
		rebase(GroupPCsLen, 0x1002B370);
		rebase(GroupListGetMemberN, 0x1002B150);
		rebase(GroupListGetLen, 0x1002B2B0);
		rebase(ObjFindInGroupArray, 0x100DF780);
		rebase(ObjIsInGroupArray, 0x100DF960);
		rebase(ObjIsAIFollower, 0x1002B220);
		rebase(ObjRemoveFromAllGroupArrays, 0x1002BD00);
		rebase(ObjAddToGroupArray, 0x100DF990);
		rebase(ObjAddToPCGroup, 0x1002BBE0);


		rebase(ObjGetSubstituteInventory, 0x1007F5B0);
		rebase(ObjGetItemAtInventoryLocation, 0x100651B0);

		rebase(ObjStatGet, 0x10074800);


		rebase(ObjFactionHas, 0x1007E430);
		rebase(ObjFactionAdd, 0x1007E480);

		rebase(XPReqForLevel, 0x100802E0);
		rebase(ObjXPGainProcess, 0x100B5480);

		rebase(sub_10152280, 0x10152280);
		rebase(CraftMagicArmsAndArmorSthg, 0x10150B20);

		rebase(ObjFeatAdd, 0x1007CF30);




		rebase(StandPointPacketGet, 0x100BDE20);
		rebase(ObjStandpointGet, 0x100BA890);
		rebase(ObjStandpointSet, 0x100BA8F0);


		rebase(ObjSpellKnownQueryGetData, 0x100762D0);
		rebase(ObjGetMaxSpellSlotLevel, 0x100765B0);


		rebase(_DoesObjectFieldExist, 0x1009C190);
		rebase(_ItemWorthFromEnhancements, 0x101509C0);

	}
private:

	// usercall... eax has field id, ecx has type
	bool(__cdecl *_DoesObjectFieldExist)();

	// usercall... eax has sthg to do with Magic Arms and Armor crafting
	bool(__cdecl *_ItemWorthFromEnhancements)();
};

extern TempleFuncs templeFuncs;


const int CONTAINER_MAX_ITEMS = 1000; // may need to be reduced
const int CRITTER_MAX_ITEMS = 24; // definitely needs to be increased in the future :)
const int CRITTER_EQUIPPED_ITEM_SLOTS = 16;
const int CRITTER_EQUIPPED_ITEM_OFFSET = 200;

void init_functions();
void init_hooks();
