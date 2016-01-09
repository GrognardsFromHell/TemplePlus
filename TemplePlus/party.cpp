#include "stdafx.h"
#include "common.h"
#include "party.h"
#include "util/config.h"
#include "util/fixes.h"
#include "critter.h"
#include "obj.h"

struct PartySystemAddresses : temple::AddressTable
{
	GroupArray * groupNpcFollowers;
	GroupArray * groupCurrentlySelected;  
	GroupArray * groupAiFollowers; 
	GroupArray * groupPcs;
	GroupArray * groupList; // the entire party (PCs, NPCs and AI followers)

	void(__cdecl * AddToCurrentlySelected)(objHndl objHnd);

	PartySystemAddresses()
	{
		rebase(AddToCurrentlySelected, 0x1002B560);
		rebase(groupNpcFollowers, 0x11E71BE0);
		rebase(groupCurrentlySelected, 0x11E71D00);
		rebase(groupAiFollowers, 0x11E71E20);
		rebase(groupPcs , 0x11E71F40);
		rebase(groupList, 0x11E721E0);
	}
	
} addresses;

PartySystem party;

class PartySystemHacks : TempleFix
{
public: 
	const char* name() override { 
		return "PartySystem Function Replacements";
	};
	void SetMaxPCs(char maxPCs);

	void apply() override 
	{
		SetMaxPCs((char)config.maxPCs);
		replaceFunction(0x1002BBE0, AddToPcGroup);
		replaceFunction(0x1002BC40, AddToNpcGroup);
	}
} partyHacks;

void PartySystemHacks::SetMaxPCs(char maxPCs)
{
	char * maxPCsBuffer = &maxPCs;
	char maxNPCs = 8 - maxPCs;
	char * maxNPCsBuffer = &maxNPCs;
	//write(0x1002BBED + 2, maxPCsBuffer , 1); // replaced with AddToPcGroup
	write(0x1002BC4D + 2, maxNPCsBuffer, 1);
	// write(0x100B0185 + 2, maxNPCsBuffer, 1); // python follower_atmax - deprecated by python reimplementation
	
}

void PartySystem::SetMaxPCs(char maxPCs)
{
	partyHacks.SetMaxPCs(maxPCs);
}

uint32_t PartySystem::AddToPCGroup(objHndl objHnd)
{
	auto npcFollowers = GroupNPCFollowersLen();
	auto pcs = GroupPCsLen();

	if ( pcs < config.maxPCs
		|| config.maxPCsFlexible && (npcFollowers + pcs < PARTY_SIZE_MAX))
	{
		auto v2 = ObjAddToGroupArray(addresses.groupPcs, objHnd);
		if (v2)
		{
			v2 = ObjAddToGroupArray(addresses.groupList, objHnd);
			if (v2)
			{
				addresses.AddToCurrentlySelected(objHnd);
			}
		}
		return v2;
	}
	return 0;
}

uint32_t PartySystem::AddToNpcGroup(objHndl objHnd)
{
	auto npcFollowers = GroupNPCFollowersLen();
	if (npcFollowers >= 5) return 0;
	
	auto pcs = GroupPCsLen();

	if (npcFollowers < PARTY_SIZE_MAX - config.maxPCs
		|| config.maxPCsFlexible && (npcFollowers + pcs < PARTY_SIZE_MAX))
	{
		auto v2 = ObjAddToGroupArray(addresses.groupNpcFollowers, objHnd);
		if (v2)
		{
			v2 = ObjAddToGroupArray(addresses.groupList, objHnd);
			if (v2)
			{
				addresses.AddToCurrentlySelected(objHnd);
			}
		}
		return v2;
	}
	return 0;
}

void PartySystem::AddToCurrentlySelected(objHndl obj)
{
	if (ObjIsInGroupArray(addresses.groupList, obj))
		ObjAddToGroupArray(addresses.groupCurrentlySelected, obj);
}

void PartySystem::GroupArrayClearMembers(GroupArray* groupArray)
{
	groupArray->GroupSize = 0;
	memset(groupArray->GroupMembers, 0, sizeof(groupArray->GroupMembers));
}

void PartySystem::CurrentlySelectedClear()
{
	GroupArrayClearMembers(addresses.groupCurrentlySelected);
}

objHndl PartySystem::GetLeader()
{
	objHndl leader = GroupListGetMemberN(0);
	if (objects.IsNPC(leader))
	{
		leader = GetConsciousPartyLeader();
	}
	return leader;
}

uint32_t AddToPcGroup(objHndl objHnd)
{
	return party.AddToPCGroup(objHnd);
}

uint32_t AddToNpcGroup(objHndl objHnd)
{
	return party.AddToNpcGroup(objHnd);
}