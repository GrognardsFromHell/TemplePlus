#include "stdafx.h"
#include "common.h"
#include "party.h"

struct PartySystemAddresses : AddressTable
{
	
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
		
	}
} partyHacks;

void PartySystemHacks::SetMaxPCs(char maxPCs)
{
	char * maxPCsBuffer = &maxPCs;
	char maxNPCs = 8 - maxPCs;
	char * maxNPCsBuffer = &maxNPCs;
	write(0x1002BBED + 2, maxPCsBuffer , 1);
	write(0x1002BC4D + 2, maxNPCsBuffer, 1);
	write(0x100B0185 + 2, maxNPCsBuffer, 1);
	
}

void PartySystem::SetMaxPCs(char maxPCs)
{
	partyHacks.SetMaxPCs(maxPCs);
}