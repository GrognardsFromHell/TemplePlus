#pragma once

#include "common.h"
#include <temple/dll.h>

#define PARTY_SIZE_MAX 8 // PCs and NPCs

struct LegacyPartySystem : temple::AddressTable
{
	void SetMaxPCs(char maxPCs);
	void GroupArraySort(GroupArray* groupArray);
	int GetPartyAlignment();
	objHndl (__cdecl*GetFellowPc)(objHndl obj); // fetches a PC who is not identical to the object. For NPCs this will try to fetch their leader.
	objHndl(__cdecl *GroupArrayMemberN)(GroupArray *, uint32_t nIdx);
	objHndl(__cdecl *GroupNPCFollowersGetMemberN)(uint32_t nIdx);
	uint32_t(__cdecl *GroupNPCFollowersLen)();
	objHndl(__cdecl *GroupPCsGetMemberN)(uint32_t nIdx);
	uint32_t(__cdecl *GroupPCsLen)();
	objHndl(__cdecl *GroupListGetMemberN)(uint32_t nIdx);
	uint32_t(__cdecl *GroupListGetLen)();
	uint32_t(__cdecl *ObjIsInGroupArray)(GroupArray *, objHndl);
	uint32_t(__cdecl *ObjIsAIFollower)(objHndl);
	bool (__cdecl *IsInParty)(objHndl critter);
	uint32_t(__cdecl * ObjFindInGroupArray)(GroupArray *, objHndl); // returns index
	uint32_t(__cdecl * ObjRemoveFromAllGroupArrays)(objHndl);
	uint32_t(__cdecl *ObjAddToGroupArray)(GroupArray *, objHndl);
	uint32_t AddToPCGroup(objHndl objHnd);
	uint32_t AddToNpcGroup(objHndl objHnd);
	void AddToCurrentlySelected(objHndl obj);
	void GroupArrayClearMembers(GroupArray * groupArray);
	void CurrentlySelectedClear();

	void (__cdecl *RumorLogAdd)(objHndl pc, int rumor);

	int (__cdecl *GetStoryState)();
	void (__cdecl *SetStoryState)(int newState);

	objHndl GetLeader();
	objHndl(__cdecl*GetConsciousPartyLeader)();
	

	LegacyPartySystem()
	{
		rebase(GetFellowPc, 0x10034A40);
		rebase(GroupArrayMemberN, 0x100DF760);
		rebase(GroupNPCFollowersGetMemberN, 0x1002B190);
		rebase(GroupNPCFollowersLen, 0x1002B360);
		rebase(GroupPCsGetMemberN, 0x1002B170);
		rebase(GroupPCsLen, 0x1002B370);
		rebase(GroupListGetMemberN, 0x1002B150);
		rebase(GroupListGetLen, 0x1002B2B0);
		rebase(GetConsciousPartyLeader, 0x1002BE60);
		rebase(ObjFindInGroupArray, 0x100DF780);
		rebase(ObjIsInGroupArray, 0x100DF960);
		rebase(ObjIsAIFollower, 0x1002B220);
		rebase(ObjRemoveFromAllGroupArrays, 0x1002BD00);
		rebase(ObjAddToGroupArray, 0x100DF990);
		//rebase(AddToPCGroup, 0x1002BBE0);
		rebase(IsInParty, 0x1002B1B0);
		rebase(RumorLogAdd, 0x1005FC20);
		rebase(GetStoryState, 0x10006A20);
		rebase(SetStoryState, 0x10006A30);
		
	}
};

extern LegacyPartySystem party;

uint32_t AddToPcGroup(objHndl objHnd);
uint32_t AddToNpcGroup(objHndl objHnd);