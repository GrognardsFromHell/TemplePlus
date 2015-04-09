#pragma once
#include "stdafx.h"
#include "common.h"

struct PartySystem : AddressTable
{
	objHndl(__cdecl *GroupArrayMemberN)(GroupArray *, uint32_t nIdx);
	objHndl(__cdecl *GroupNPCFollowersGetMemberN)(uint32_t nIdx);
	uint32_t(__cdecl *GroupNPCFollowersLen)();
	objHndl(__cdecl *GroupPCsGetMemberN)(uint32_t nIdx);
	uint32_t(__cdecl *GroupPCsLen)();
	objHndl(__cdecl *GroupListGetMemberN)(uint32_t nIdx);
	uint32_t(__cdecl *GroupListGetLen)();
	uint32_t(__cdecl *ObjIsInGroupArray)(GroupArray *, objHndl);
	uint32_t(__cdecl *ObjIsAIFollower)(objHndl);
	uint32_t(__cdecl * ObjFindInGroupArray)(GroupArray *, objHndl); // returns index
	uint32_t(__cdecl * ObjRemoveFromAllGroupArrays)(objHndl);
	uint32_t(__cdecl *ObjAddToGroupArray)(GroupArray *, objHndl);
	uint32_t(__cdecl *ObjAddToPCGroup)(objHndl);

	PartySystem()
	{
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
	}
};

extern PartySystem party;