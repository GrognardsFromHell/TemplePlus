#pragma once

#include "common.h"

#define NUM_FEATS 664 // inc. those hacked by Moebius/SpellSlinger (otherwise vanilla is 649)


struct FeatSystem : AddressTable
{	/* feat property bit meaning:
	0x00000001 - can gain multiple times
	0x00000002 - Not implemented (cannot select)
	0x00000004 - Race Automatic Feat
	0x00000008 - Class Automatic Feat
	0x00000010 - Fighter feat
	0x00000020 - Monk 1st lvl
	0x00000040 - Monk 2nd lvl feat
	0x00000080 - Monk 6th lvl feat
	0x00000100 - part of a multiselect feat apparently (though not quite always - Spell Focus seems like an exception - or was it a multiselect?)
	0x00000300 - EWP
	0x00000500 - Imp. Crit
	0x00000900 - MWP
	0x00001100 - Skill Focus
	0x00002100 - Wpn Finesse
	0x00004100 - Wpn Focus
	0x00008100 - Wpn Spec.
	0x00010100 - G. Wpn Focus
	0x00020000 - Wizard Feat (crafting, metamagic etc.)
	0x00040000 - Rogue 10th lvl Feat
	*/
	uint32_t * featPropertiesTable;
	uint32_t * classFeatTable;
	uint32_t * featPreReqTable;
	objHndl * charEditorObjHnd;
	Stat * charEditorClassCode;

	uint32_t racialFeats[ 10 * NUM_RACES ];
	uint32_t HasFeatCount(objHndl objHnd, feat_enums featEnum);
	uint32_t HasFeatCountByClass(objHndl objHnd, feat_enums featEnum, Stat classEnum, uint32_t rangerSpecializationFeat);
	uint32_t FeatListElective(objHndl objHnd, feat_enums * listOut);
	uint32_t FeatListGet(objHndl objHnd, feat_enums * listOut, Stat classBeingLevelled, feat_enums rangerSpecFeat);
	uint32_t FeatExistsInArray(feat_enums featCode, feat_enums * featArray, uint32_t featArrayLen);
	uint32_t WeaponFeatCheck(objHndl objHnd, feat_enums * featArray, uint32_t featArrayLen, Stat classBeingLeveled, WeaponTypes wpnType);

	vector<feat_enums> GetFeats(objHndl handle); // This is what objHndl.feats in python returns ??

	uint32_t rangerArcheryFeats[3 * 2];
	uint32_t rangerTwoWeaponFeats[3 * 2];


	void(__cdecl *ToEE_WeaponFeatCheck)();
	uint32_t(__cdecl *FeatAdd)(objHndl, feat_enums);


	FeatSystem();
};

extern FeatSystem feats;



uint32_t _HasFeatCount(objHndl objHnd, feat_enums featEnum);
uint32_t _HasFeatCountByClass(objHndl objHnd, feat_enums featEnum, Stat classLevelBeingRaised, uint32_t rangerSpecializationFeat);
uint32_t _FeatExistsInArray(feat_enums featCode, feat_enums * featArray, uint32_t featArrayLen);
uint32_t _FeatPrereqsCheck(objHndl, feat_enums featIdx, feat_enums*, uint32_t, Stat, Stat);
uint32_t _RogueSpecialFeat(feat_enums featIdx, uint32_t newLevel);
uint32_t _FeatListGet(objHndl objHnd, feat_enums * listOut, Stat classBeingLevelled, feat_enums rangerSpecFeat);
uint32_t _FeatListElective(objHndl objHnd, feat_enums * listOut);
uint32_t _WeaponFeatCheck(objHndl objHnd, feat_enums * featArray, uint32_t featArrayLen, Stat classBeingLeveled, WeaponTypes wpnType);
uint32_t _WeaponFeatCheckSimpleWrapper(objHndl objHnd, WeaponTypes wpnType);