#include "stdafx.h"
#include "common.h"
#include "party.h"
#include "util/config.h"
#include "util/fixes.h"

struct LegacyPartySystemAddresses : temple::AddressTable
{
	GroupArray * groupNpcFollowers;
	GroupArray * groupCurrentlySelected;  
	GroupArray * groupAiFollowers; 
	GroupArray * groupPcs;
	GroupArray * groupList; // the entire party (PCs, NPCs and AI followers)

	void(__cdecl * AddToCurrentlySelected)(objHndl objHnd);

	LegacyPartySystemAddresses()
	{
		rebase(AddToCurrentlySelected, 0x1002B560);
		rebase(groupNpcFollowers, 0x11E71BE0);
		rebase(groupCurrentlySelected, 0x11E71D00);
		rebase(groupAiFollowers, 0x11E71E20);
		rebase(groupPcs , 0x11E71F40);
		rebase(groupList, 0x11E721E0);
	}
	
} addresses;

LegacyPartySystem party;

class LegacyPartySystemHacks : TempleFix
{
public: 
	const char* name() override { 
		return "LegacyPartySystem Function Replacements";
	};
	void SetMaxPCs(char maxPCs);

	void apply() override 
	{
		SetMaxPCs((char)config.maxPCs);
		replaceFunction(0x1002BBE0, AddToPcGroup);
		replaceFunction(0x1002BC40, AddToNpcGroup);
	}
} partyHacks;

void LegacyPartySystemHacks::SetMaxPCs(char maxPCs)
{
	char * maxPCsBuffer = &maxPCs;
	char maxNPCs = 8 - maxPCs;
	char * maxNPCsBuffer = &maxNPCs;
	//write(0x1002BBED + 2, maxPCsBuffer , 1); // replaced with AddToPcGroup
	write(0x1002BC4D + 2, maxNPCsBuffer, 1);
	// write(0x100B0185 + 2, maxNPCsBuffer, 1); // python follower_atmax - deprecated by python reimplementation
	
}

void LegacyPartySystem::SetMaxPCs(char maxPCs)
{
	partyHacks.SetMaxPCs(maxPCs);
}

uint32_t LegacyPartySystem::AddToPCGroup(objHndl objHnd)
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

uint32_t LegacyPartySystem::AddToNpcGroup(objHndl objHnd)
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

uint32_t AddToPcGroup(objHndl objHnd)
{
	return party.AddToPCGroup(objHnd);
}

uint32_t AddToNpcGroup(objHndl objHnd)
{
	return party.AddToNpcGroup(objHnd);
}