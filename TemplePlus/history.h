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
	int CreateRollHistoryLineFromMesfile(int historyMesLine, objHndl obj, objHndl obj2);
	int RollHistoryType4Add(objHndl obj, int dc, const char* text, uint32_t dicePacked, int d20RollRes, BonusList* bonlist);
	int RollHistoryType7Add(objHndl obj, BonusList* bonlist, int line, int unk);
	int RollHistoryType0Add(int RollResult, int unk, objHndl Obj, objHndl Obj2, BonusList* Bonlist, BonusList* Bonlist2, D20CAF flags);
	int RollHistoryType5Add(objHndl obj, objHndl tgt, int defenderMissChance, int combatMesLine, int missChanceRoll, int combatMesLineResult, int combatMesLineCheckType);
	static void CreateFromFreeText(const char *);

	HistoryEntry* HistoryFind(int histId);
	void AppendHistoryId(int histId);
};

extern HistorySystem histSys;
