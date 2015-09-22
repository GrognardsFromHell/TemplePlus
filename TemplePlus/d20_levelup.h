#pragma once
#include "stdafx.h"
#include "common.h"
#include "idxtables.h"

#pragma pack(push, 4)
struct LevelupPacket
{
	int flags;
	int classCode;
	Stat abilityScoreRaised;
	feat_enums feats[4];
	int skillPointsAdded[42]; // array keeping track of how many skill pts were added to each skill
	int spellEnumCount; // how many spells were learned (added to spells_known)
	int spellEnums[802];
	int spellEnumToRemove; // spell removed (for Sorcerers)
};
const auto TestSizeOfLevelupPacket = sizeof(LevelupPacket); // should be 3412 (0xD54)

struct LevelupTabEntry
{
	int nameHash; //hash of the entry name from levelup.tab
	Stat classes[6]; // specs for which classes to advance, in sequence I think (e.g. Fighter, Fighter, Rogue to achieve Rog X / Ftr 2X
	Stat attributes[3]; // which attributes to raise (I think it only uses the first value? not sure)
	feat_enums feats[10]; // which feats to get
	int skills[4]; // which skills to raise
	int skills2ndPriority[5]; // if there are points left
	SpellStoreData spells[16]; // what spells to learn
	int terminator;
	int pad[3];
};
const auto TestSizeOfLevelupTabEntry = sizeof(LevelupTabEntry); // should be 644 (0x284)

struct LevelupState
{
	LevelupTabEntry *lvlupTab;
	int classCodeIdx; // idx for the array of classes in LevelupTabEntry
	int16_t data1;
	int16_t lastClassCodeIdx;
};
const auto TestSizeOfLevelupState = sizeof(LevelupState); // should be 12 (0xC)
#pragma pack(pop)