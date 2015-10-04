#pragma once

#include "common.h"
#include <temple/dll.h>

struct HistoryEntry;
struct HistoryArrayEntry;

struct HistorySystem : temple::AddressTable
{
	int32_t(__cdecl * RollHistoryAdd)(HistoryEntry * hist);
	// todo: the annoying AppendHistoryId
	int RollHistoryAddType6OpposedCheck(objHndl attacker, objHndl defender, int attackerRoll, int defenderRoll, BonusList* attackerBonlist, BonusList* defenderBonlist, int combatMesLineTitle, int combatMesLineResult, int sthg); // 10047F70
	int CreateRollHistoryString(int histId);// 100DFFF0
	HistorySystem();
};

extern HistorySystem histSys;
