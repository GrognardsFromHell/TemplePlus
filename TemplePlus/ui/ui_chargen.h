#pragma once
#include "common.h"


struct CharEditorSelectionPacket {
	int abilityStats[6];
	int numRerolls; // number of rerolls
	int isPointbuy;
	char rerollString[128];
	Stat statBeingRaised;
	int raceId; // 7 is considered invalid
	int genderId; // 2 is considered invalid
	int height0;
	int height1;
	int modelScale;
	int hair0;
	int hair1;
	Stat classCode;
	int deityId;
	int domain1;
	int domain2;
	Alignment alignment;
	int alignmentChoice; // 1 is for Positive Energy, 2 is for Negative Energy
	feat_enums feat0;
	feat_enums feat1;
	feat_enums feat2;
	int skillPointsAdded[42]; // idx corresponds to skill enum
	int skillPointsSpent;
	int availableSkillPoints;
	int spellEnums[SPELL_ENUM_MAX_VANILLA];
	int spellEnumsAddedCount;
	int spellEnumToRemove; // for sorcerers who swap out spells
	int wizSchool;
	int forbiddenSchool1;
	int forbiddenSchool2;
	feat_enums feat3;
	feat_enums feat4;
	int portraitId;
	char voiceFile[256];
	int voiceId; // -1 is considered invalid
};


struct FeatInfo {
	int featEnum;
	int flag = 0;
	int minLevel = 1;
	FeatInfo(int FeatEnum) : featEnum(FeatEnum) {};
	FeatInfo(std::string &featName);
};