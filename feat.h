#pragma once

#include "common.h"

#define NUM_FEATS 664


struct FeatSystem : AddressTable
{
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

	uint32_t rangerArcheryFeats[2 * 2];
	uint32_t rangerTwoWeaponFeats[2 * 2];


	void(__cdecl *ToEE_WeaponFeatCheck)();

	FeatSystem();
};

extern FeatSystem feats;



uint32_t _HasFeatCount(objHndl objHnd, feat_enums featEnum);
uint32_t _HasFeatCountByClass(objHndl objHnd, feat_enums featEnum, Stat classLevelBeingRaised, uint32_t rangerSpecializationFeat);
uint32_t _FeatExistsInArray(feat_enums featCode, feat_enums * featArray, uint32_t featArrayLen);
uint32_t _FeatPrereqsCheck(objHndl, feat_enums featIdx, feat_enums*, uint32_t, Stat, Stat);
uint32_t _RogueSpecialFeat(feat_enums featIdx, uint32_t newLevel);
uint32_t FeatListGet(objHndl objHnd, feat_enums * listOut, Stat classBeingLevelled, feat_enums rangerSpecFeat);
uint32_t _FeatListElective(objHndl objHnd, feat_enums * listOut);
uint32_t _WeaponFeatCheck(objHndl objHnd, feat_enums * featArray, uint32_t featArrayLen, Stat classBeingLeveled, WeaponTypes wpnType);