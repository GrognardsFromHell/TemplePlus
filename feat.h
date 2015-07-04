#pragma once

#include "common.h"

#define NUM_FEATS 747 // inc. those hacked by Moebius/SpellSlinger (otherwise vanilla is 649)
#include "tig/tig_mes.h"
//664th - FEAT_GREATER_TWO_WEAPON_FIGHTING_RANGER

struct FeatPrereqRow;
struct ClassFeatTableEntry
{
	feat_enums feat;
	int minLvl;
};
struct ClassFeatTableRow
{
	ClassFeatTableEntry entries[20];
}; // all the rows
struct ClassFeatTable
{
	ClassFeatTableRow classEntries[NUM_CLASSES];
};

struct TabFileStatus;

extern TabFileStatus featPropertiesTabFile;
extern uint32_t  featPropertiesTable[];
extern FeatPrereqRow featPreReqTable[];
extern MesHandle * featMes;
extern MesHandle * featEnumsMes;
extern char ** featNames;

struct FeatPrereq
{
	int featPrereqCode;
	int featPrereqCodeArg;
};

struct FeatPrereqRow
{
	FeatPrereq featPrereqs[8];
};



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
	0x00080000 - Multiselect Parent   NEW (overrides disablement for particular display purposes)
	0x00100100 - G. Wpn. Specialization   (add 0x10 to make it a fighter feat, so the property number should be 1048848)
	*/
	uint32_t * m_featPropertiesTable;
	FeatPrereqRow * m_featPreReqTable;
	char * featNames[NUM_FEATS +1000];
	MesHandle * featMes;
	MesHandle * featEnumsMes;
	TabFileStatus * featTabFile;
	//static TabFileStatus featPropertiesTabFile;
	ClassFeatTable * classFeatTable;
	objHndl * charEditorObjHnd;
	Stat * charEditorClassCode;

	uint32_t racialFeats[ 10 * NUM_RACES ];
	uint32_t HasFeatCount(objHndl objHnd, feat_enums featEnum);
	uint32_t HasFeatCountByClass(objHndl objHnd, feat_enums featEnum, Stat classEnum, uint32_t rangerSpecializationFeat);
	uint32_t FeatListElective(objHndl objHnd, feat_enums * listOut);
	uint32_t FeatListGet(objHndl objHnd, feat_enums * listOut, Stat classBeingLevelled, feat_enums rangerSpecFeat);
	uint32_t FeatExistsInArray(feat_enums featCode, feat_enums * featArray, uint32_t featArrayLen);
	uint32_t WeaponFeatCheck(objHndl objHnd, feat_enums * featArray, uint32_t featArrayLen, Stat classBeingLeveled, WeaponTypes wpnType);
	uint32_t FeatPrereqsCheck(objHndl objHnd, feat_enums featIdx, feat_enums * featArray, uint32_t featArrayLen, Stat classCodeBeingLevelledUp, Stat abilityScoreBeingIncreased);

	vector<feat_enums> GetFeats(objHndl handle); // This is what objHndl.feats in python returns ??
	char* GetFeatName(feat_enums feat);
	int IsFeatEnabled(feat_enums feat);
	int IsMagicFeat(feat_enums feat); // crafting / metamagic feats (that Wiz/Sorcs can pick as bonus feats)
	int IsFeatPartOfMultiselect(feat_enums feat); // hidden feats that are only selectable in a submenu
	int IsFeatRacialOrClassAutomatic(feat_enums feat);  // feats automatically granted (cannot be manually selected at levelup)
	int IsClassFeat(feat_enums feat);
	int IsFighterFeat(feat_enums feat); // feats that fighters can select as bonus feats
	int IsFeatPropertySet(feat_enums feat, int featProp); // checks bitfield if the entire featProp is set (i.e. partial matches return 0)
	uint32_t rangerArcheryFeats[4 * 2 + 100];
	uint32_t rangerTwoWeaponFeats[4 * 2 + 100];


	void(__cdecl *ToEE_WeaponFeatCheck)();
	uint32_t(__cdecl *FeatAdd)(objHndl, feat_enums);

	uint32_t(__cdecl *featTabLineParser)(TabFileStatus*, uint32_t, const char**);
	FeatSystem();
};

extern FeatSystem feats;

int FeatInit();

uint32_t _HasFeatCount(objHndl objHnd, feat_enums featEnum);
uint32_t _HasFeatCountByClass(objHndl objHnd, feat_enums featEnum, Stat classLevelBeingRaised, uint32_t rangerSpecializationFeat);
uint32_t _FeatExistsInArray(feat_enums featCode, feat_enums * featArray, uint32_t featArrayLen);
uint32_t _FeatPrereqsCheck(objHndl, feat_enums featIdx, feat_enums*, uint32_t, Stat, Stat);
uint32_t _RogueSpecialFeat(feat_enums featIdx, uint32_t newLevel);
uint32_t _FeatListGet(objHndl objHnd, feat_enums * listOut, Stat classBeingLevelled, feat_enums rangerSpecFeat);
uint32_t _FeatListElective(objHndl objHnd, feat_enums * listOut);
uint32_t _WeaponFeatCheck(objHndl objHnd, feat_enums * featArray, uint32_t featArrayLen, Stat classBeingLeveled, WeaponTypes wpnType);
uint32_t _WeaponFeatCheckSimpleWrapper(objHndl objHnd, WeaponTypes wpnType);

const char * _GetFeatName(feat_enums feat);
int _IsFeatEnabled(feat_enums feat);
int _IsMagicFeat(feat_enums feat);
int _IsFeatPartOfMultiselect(feat_enums feat);
int _IsFeatRacialOrClassAutomatic(feat_enums feat);
int _IsClassFeat(feat_enums feat);
int _IsFighterFeat(feat_enums feat);
int  _IsExoticWeaponProfFeat(feat_enums feat);
int _IsImprovedCriticalFeat(feat_enums feat);
int _IsMartialWeaponFeat(feat_enums feat);
int _IsSkillFocusFeat(feat_enums feat);
int _IsWeaponFinesseFeat(feat_enums feat);
int _IsWeaponFocusFeat(feat_enums feat);
int _IsGreaterWeaponFocusFeat(feat_enums feat);
int _IsWeaponSpecializationFeat(feat_enums feat);



uint32_t featPropertiesTabLineParser(TabFileStatus*, uint32_t, const char**);