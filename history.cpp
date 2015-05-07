#include "stdafx.h"
#include "common.h"
#include "history.h"
#include "damage.h"


class HistSysReplacements : public TempleFix
{
	macTempleFix(History System)
	{
		
	}
} historySysReplacements;


#pragma region History System Implementation
HistorySystem histSys;


HistorySystem::HistorySystem()
{
	rebase(RollHistoryAdd, 0x10047430);
}
#pragma endregion

#pragma region Replacements


#pragma endregion


struct HistoryEntry
{
	uint32_t histId;
	uint32_t histType;
	objHndl obj;
	ObjectId objId;
	char objDescr[2000];
	objHndl obj2;
	ObjectId obj2Id;
	char obj2Descr[2000];
	uint32_t prevId;
	uint32_t nextId;
};

struct HistoryEntryType1 : HistoryEntry
{
	DamagePacket dmgPkt;
};


struct HistoryEntryType2 : HistoryEntry
{
	uint32_t dicePacked;
	int32_t rollResult;
	uint32_t skillIdx;
	uint32_t dc;
	BonusList bonlist;
	uint32_t pad[100];
};

const auto TestSizeOfHistoryEntryType1 = sizeof(HistoryEntryType1); // should be 5384 (0x1508)
const auto TestSizeOfHistoryEntryType2 = sizeof(HistoryEntryType2); // should be 5384 (0x1508)