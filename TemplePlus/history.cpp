#include "stdafx.h"
#include "common.h"
#include "history.h"
#include "damage.h"
#include "tig/tig_mes.h"
#include "util/fixes.h"

class HistSysReplacements : public TempleFix
{
public: 
	const char* name() override { return "History System" "Function Replacements";} 
	void apply() override 
	{
		
	}
} historySysReplacements;


struct HistorySystemAddresses : temple::AddressTable
{

	int32_t *rollSerialNumber; 
	MesHandle * rollUiMesHandle;
	HistoryArrayEntry * histArray;
	int (__cdecl *RollHistoryType1Add)(objHndl objHnd, objHndl objHnd2, DamagePacket *damPkt);
	int(__cdecl *RollHistoryType2Add)(objHndl objHnd, objHndl objHnd2, int skillIdx, int dicePacked, int rollResult, int DC, BonusList *bonlist);
	int (__cdecl *RollHistoryType3Add)(objHndl obj, int DC, int saveType, int flags, int dicePacked, int rollResult, BonusList *bonListIn);
	int (__cdecl *RollHistoryAddType6OpposedCheck)(objHndl attacker, objHndl defender, int attackerRoll, int defenderRoll, BonusList* attackerBonlist, BonusList* defenderBonlist, int combatMesLineTitle, int combatMesLineResult, int flag);
	int (__cdecl*CreateRollHistoryString)(int histId);
	int(__cdecl*CreateRollHistoryLineFromMesfile)(int historyMesLine, objHndl obj, objHndl obj2);
	HistorySystemAddresses()
	{

		rebase(RollHistoryType1Add, 0x10047C80);
		rebase(RollHistoryType2Add, 0x10047CF0);
		rebase(RollHistoryType3Add, 0x10047D90);
		rebase(RollHistoryAddType6OpposedCheck, 0x10047F70);
		rebase(CreateRollHistoryString, 0x100DFFF0);
		rebase(rollUiMesHandle, 0x102B0168);
		rebase(rollSerialNumber, 0x102B016C);
		rebase(histArray, 0x109DDA20);
		rebase(CreateRollHistoryLineFromMesfile, 0x100E01F0);
	}
} addresses;



#pragma region History System Implementation
HistorySystem histSys;


int HistorySystem::RollHistoryAddType6OpposedCheck(objHndl attacker, objHndl defender, int attackerRoll, int defenderRoll, BonusList* attackerBonlist, BonusList* defenderBonlist, int combatMesLineTitle, int combatMesLineResult, int flag)
{
	return addresses.RollHistoryAddType6OpposedCheck(attacker, defender, attackerRoll, defenderRoll, attackerBonlist, defenderBonlist, combatMesLineTitle, combatMesLineResult, flag);
}

int HistorySystem::CreateRollHistoryString(int histId)
{
	return addresses.CreateRollHistoryString(histId);
}

HistorySystem::HistorySystem()
{
	rebase(RollHistoryAdd, 0x10047430);
}

int HistorySystem::CreateRollHistoryLineFromMesfile(int historyMesLine, objHndl obj, objHndl obj2)
{
	return addresses.CreateRollHistoryLineFromMesfile(historyMesLine, obj, obj2);
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

struct HistoryArrayEntry
{
	uint32_t field0;
	uint32_t field4;
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
	uint32_t pad[326];
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

struct HistoryEntryType3 : HistoryEntry
{
	uint32_t dicePacked;
	int32_t rollResult;
	uint32_t dc;
	uint32_t saveType;
	uint32_t saveFlags;
	BonusList bonlist;
	uint32_t pad[99];
};

const auto TestSizeOfHistoryEntryType1 = sizeof(HistoryEntryType1); // should be 5384 (0x1508)
const auto TestSizeOfHistoryEntryType2 = sizeof(HistoryEntryType2); // should be 5384 (0x1508)
const auto TestSizeOfHistoryEntryType3 = sizeof(HistoryEntryType3); // should be 5384 (0x1508)
const auto TestSizeOfHistoryArrayEntry = sizeof(HistoryArrayEntry); // should be 5392 (0x1510)