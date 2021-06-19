#pragma once

#include "common.h"
#include <temple/dll.h>

struct HistoryEntry;
struct HistoryArrayEntry;
struct D20RollHistoryEntry;
struct DamagePacket;

struct HistorySystem : temple::AddressTable
{
	int32_t(__cdecl * RollHistoryAdd)(HistoryEntry * hist);
	// todo: the annoying AppendHistoryId
	int RollHistoryAddType6OpposedCheck(objHndl attacker, objHndl defender, int attackerRoll, int defenderRoll, BonusList* attackerBonlist, BonusList* defenderBonlist, int combatMesLineTitle, int combatMesLineResult, int sthg); // 10047F70 Opposed Check Roll
	int CreateRollHistoryString(int histId);// 100DFFF0
	HistorySystem();
	int CreateRollHistoryLineFromMesfile(int historyMesLine, objHndl obj, objHndl obj2);
	int RollHistoryAddType1DamageRoll(objHndl attacker, objHndl tgt, DamagePacket* dmg);
	int RollHistoryAddType2SkillRoll(objHndl obj, uint32_t dicePacked, int rollResult, int dc, int skillIdx, BonusList* bonlist); // Skill Check
	int RollHistoryAddType3SavingThrow(objHndl handle, int dc, SavingThrowType saveType, int flags, uint32_t dicePacked, int d20RollRes, BonusList* bonlist); // Saving Throw
	int RollHistoryAddType4MiscCheckRoll(objHndl obj, int dc, const char* text, uint32_t dicePacked, int d20RollRes, BonusList* bonlist); // MiscCheckRoll
	int RollHistoryAddType7MiscBonus(objHndl obj, BonusList* bonlist, int line, int unk); // Misc Bonus
	int RollHistoryAddType0AttackRoll(int RollResult, int unk, objHndl Obj, objHndl Obj2, BonusList* Bonlist, BonusList* Bonlist2, D20CAF flags); // Attack roll
	int RollHistoryAddType5PercentChanceRoll(objHndl obj, objHndl tgt, int defenderMissChance, int combatMesLine, int missChanceRoll, int combatMesLineResult, int combatMesLineCheckType); // Percent Chance Roll

	// 1 - Damage Roll
	// 2 - Skill Roll
	// 3 - Saving Throw


	static void CreateFromFreeText(const char *);
	void PrintSpellCast(objHndl caster, int spEnum); // [ACTOR] casts [SPELL NAME]!

	HistoryEntry* HistoryFind(int histId);
	void AppendHistoryId(int histId);

	void ParseHistoryEntry(int histId, D20RollHistoryEntry* rh);
	const char* GetRollUiString(int lineId);
	const char* GetHistoryMesLine(int lineId);

	D20RollHistoryEntry &GetD20RollHistoryConsole();
};

extern HistorySystem histSys;
