#pragma once
#include "common.h"
#include <spell_structs.h>

struct GameSystemConf;

struct CharEditorSelectionPacket{
	Stat abilityStats[6];
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
	int alignmentChoice;
	feat_enums feat0;
	feat_enums feat1;
	feat_enums feat2;
	int skillPointsAdded[42]; // idx corresponds to skill enum
	int skillPointsSpent;
	int availableSkillPoints;
	int spellEnums[SPELL_ENUM_MAX];
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


struct CharEditorSystem{
	const char* name;
	BOOL(__cdecl *systemInit)(GameSystemConf *);
	BOOL(__cdecl *systemReset)(void *);
	int pad; // possibly some unused callback?
	void(__cdecl *free)();
	void(__cdecl *hide)();
	void(__cdecl *show)();
	int(__cdecl *checkComplete)(); // checks if the char editing stage is complete (thus allowing you to move on to the next stage). This is checked at every render call.
	int(__cdecl*debugMaybe)();
	void(__cdecl *reset)(CharEditorSelectionPacket & editSpec);
	BOOL(__cdecl *activate)(); // inits values and sets appropriate states for buttons based on gameplay logic (e.g. stuff exclusive to certain classes etc.)
};

const auto testSizeOfCharEditorSelectionPacket = sizeof(CharEditorSelectionPacket); // should be 3640 (0xE38)



int HookedFeatMultiselectSub_101A8080(feat_enums feat);