#pragma once
#include "stdafx.h"
#include "common.h"


struct CharEditorSelectionPacket
{
	Stat abilityStats[6];
	int numRerolls;
	int pointBuySthg;
	char rerollString[128];
	Stat statBeingRaised;
	int raceSthg;
	int genderSthg;
	int field_AC;
	int field_B0;
	int field_B4;
	int field_B8;
	int field_BC;
	Stat classCode;
	int field_C4;
	int domain1;
	int domain2;
	int field_D0;
	int alignmentChoice;
	feat_enums feat0;
	feat_enums feat1;
	feat_enums feat2;
	int skillPointsAdded[42]; // idx corresponds to skill
	int skillPointsSpent;
	int availableSkillPoints;
	int spellEnums[802];
	int spellEnumsAddedCount;
	int spellEnumToRemove; // for sorcerers who swap out spells
	int wizSchool;
	int forbiddenSchool1;
	int forbiddenSchool2;
	feat_enums feat3;
	feat_enums feat4;
};

const auto testSizeOfCharEditorSelectionPacket = sizeof(CharEditorSelectionPacket); // should be 3640 (0xE38)



int HookedFeatMultiselectSub_101A8080(feat_enums feat);