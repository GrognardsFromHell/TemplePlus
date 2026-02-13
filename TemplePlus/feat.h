#pragma once

#include "common.h"

#define NUM_FEATS 750 // vanilla was 649 (and Moebius hack increased this to 664 I think)
#include "tig/tig_mes.h"
#include <map>
#include <unordered_set>

enum FeatPropertyFlag : uint32_t {
	FPF_CAN_GAIN_MULTIPLE_TIMES = 0x1,
	FPF_DISABLED = 0x2,
	FPF_RACE_AUTOMATIC = 0x4,
	FPF_CLASS_AUTMATIC = 0x8 ,
	FPF_FIGHTER_BONUS = 0x10,
	FPF_MONK_BONUS_1st = 0x20,
	FPF_MONK_BONUS_2nd = 0x40,
	FPF_MONK_BONUS_6th = 0x80,
	FPF_MULTI_SELECT_ITEM = 0x100,
	FPF_EXOTIC_WEAP_ITEM = 0x300,
	FPF_IMPR_CRIT_ITEM = 0x500,
	FPF_MARTIAL_WEAP_ITEM = 0x900,
	FPF_SKILL_FOCUS_ITEM = 0x1100,
	FPF_WEAP_FINESSE_ITEM = 0x2100,
	FPF_WEAP_FOCUS_ITEM = 0x4100,
	FPF_WEAP_SPEC_ITEM = 0x8100,
	FPF_GREATER_WEAP_FOCUS_ITEM = 0x10100,
	FPF_WIZARD_BONUS = 0x20000,
	FPF_ROGUE_BONUS = 0x40000, // rogue bonus at 10th level

	FPF_MULTI_MASTER = 0x80000, // head of multiselect class of feats (NEW)
	FPF_GREAT_WEAP_SPEC_ITEM = 0x100100, // NEW	
	FPF_PSIONIC = 0x200000,
	FPF_NON_CORE =0x400000,
	FPF_CUSTOM_REQ = 0x800000, // signifies that requirements should be checked via script
	FPF_POWER_CRIT_ITEM = 0x1000000
};


struct FeatPrereqRow;
struct ClassFeatTableEntry
{
	feat_enums feat;
	int minLvl;
};
struct ClassFeatTableRow
{
	ClassFeatTableEntry entries[40];
}; // all the rows
struct ClassFeatTable
{
	ClassFeatTableRow classEntries[VANILLA_NUM_CLASSES];
};


struct FeatPrereq
{
	int featPrereqCode; // see feat_requirement_codes
	int featPrereqCodeArg;
};

struct FeatPrereqRow
{
	FeatPrereq featPrereqs[8];
};

struct NewFeatSpec {
	FeatPropertyFlag flags;
	std::string name;
	std::string description;
	std::string prereqDescr;
	std::vector<FeatPrereq> prereqs;
	feat_enums parentId = (feat_enums)0;              // for multiselect feats such as Weapon Focus
	std::vector<feat_enums> children; // for multiselect feats such as Weapon Focus
	WeaponTypes weapType = wt_none;      // for weapon feats which are weapon specific (such as Weapon Focus - Shortsword)
	NewFeatSpec() { flags = (FeatPropertyFlag)0; };
};

struct TabFileStatus;

extern TabFileStatus featPropertiesTabFile;
extern uint32_t  featPropertiesTable[];
extern FeatPrereqRow featPreReqTable[];
extern MesHandle * featMes;
extern MesHandle * featEnumsMes;
// extern char ** featNames;




struct LegacyFeatSystem : temple::AddressTable
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
	std::vector<feat_enums> newFeats;
	uint32_t * m_featPropertiesTable;
	FeatPrereqRow * m_featPreReqTable;
	char * featNames[NUM_FEATS +1000];
	MesHandle * featMes;
	MesHandle featMesNew; // for the new stuff
	MesHandle * featEnumsMes;
	TabFileStatus * featTabFile;
	//static TabFileStatus featPropertiesTabFile;
	ClassFeatTable * classFeatTable;
	objHndl * charEditorObjHnd;
	Stat * charEditorClassCode;
	char emptyString[1000] ;
	char featPrereqDescrBuffer[5000];
	std::unordered_set<feat_enums> metamagicFeats;

	uint32_t HasFeatCount(objHndl objHnd, feat_enums featEnum);
	uint32_t HasFeatCountByClass(objHndl objHnd, feat_enums featEnum, Stat classEnum, uint32_t rangerSpecializationFeat, uint32_t newDomain1, uint32_t newDomain2, uint32_t alignmentChoiceNew);
	uint32_t HasFeatCountByClass(objHndl objHnd, feat_enums featEnum, Stat classEnum, uint32_t rangerSpecializationFeat);
	uint32_t HasFeatCountByClass(objHndl objHnd, feat_enums featEnum);
	bool HasMetamagicFeat(objHndl handle);

	uint32_t FeatListElective(objHndl objHnd, feat_enums * listOut);
	uint32_t FeatListGet(objHndl objHnd, feat_enums * listOut, Stat classBeingLevelled, feat_enums rangerSpecFeat);
	uint32_t FeatExistsInArray(feat_enums featCode, feat_enums * featArray, uint32_t featArrayLen);
	uint32_t FeatPrereqsCheck(objHndl objHnd, feat_enums featIdx, feat_enums * featArray, uint32_t featArrayLen, Stat classCodeBeingLevelledUp, Stat abilityScoreBeingIncreased);

	std::vector<feat_enums> GetFeats(objHndl handle); // This is what objHndl.feats in python returns ??
	char* GetFeatName(feat_enums feat);
	char* GetFeatDescription(feat_enums feat);
	char* GetFeatPrereqDescription(feat_enums feat);
	const char* GetFeatHelpTopic(feat_enums feat);
	int IsFeatEnabled(feat_enums feat);
	int IsMagicFeat(feat_enums feat); // crafting / metamagic feats (that Wiz/Sorcs can pick as bonus feats)
	
	//Metamgic feats (subset of magic feats)
	void AddMetamagicFeat(feat_enums feat);
	bool IsMetamagicFeat(feat_enums feat);
	std::vector<feat_enums> GetMetamagicFeats();

	int IsFeatRacialOrClassAutomatic(feat_enums feat);  // feats automatically granted (cannot be manually selected at levelup)
	int IsClassFeat(feat_enums feat);
	int IsFighterFeat(feat_enums feat); // feats that fighters can select as bonus feats
	int IsFeatPropertySet(feat_enums feat, int featProp); // checks bitfield if the entire featProp is set (i.e. partial matches return 0)

	// weapon feats handling
	WeaponTypes GetWeaponType(feat_enums feat); // gets the associated weapon type
	uint32_t WeaponFeatCheck(objHndl objHnd, feat_enums * featArray, uint32_t featArrayLen, Stat classBeingLeveled, WeaponTypes wpnType);
	feat_enums GetFeatForWeaponType(WeaponTypes wt, feat_enums baseFeat = FEAT_NONE); // for stuff like Weapon Specialization/Focus etc.

	int IsFeatPartOfMultiselect(feat_enums feat); // hidden feats that are only selectable in a submenu
	bool IsFeatMultiSelectMaster(feat_enums feat);
	feat_enums MultiselectGetFirst(feat_enums feat);
	void MultiselectGetChildren(feat_enums feat, std::vector<feat_enums>& out);

	bool IsNonCore(feat_enums feat);
	void DoForAllFeats(void (__cdecl*cb)(int featEnum));
	
	uint32_t rangerArcheryFeats[4 * 2 + 100] =
		{ FEAT_RANGER_RAPID_SHOT, 2,
			FEAT_RANGER_MANYSHOT, 6,
			FEAT_IMPROVED_PRECISE_SHOT_RANGER, 11,
			static_cast<uint32_t>(-1), static_cast<uint32_t>(-1)
		};
	uint32_t rangerTwoWeaponFeats[4 * 2 + 100] =
		{ FEAT_TWO_WEAPON_FIGHTING_RANGER, 2,
			FEAT_IMPROVED_TWO_WEAPON_FIGHTING_RANGER, 6,
			FEAT_GREATER_TWO_WEAPON_FIGHTING_RANGER, 11,
			static_cast<uint32_t>(-1), static_cast<uint32_t>(-1)
		};


	void(__cdecl *ToEE_WeaponFeatCheck)();
	void FeatAdd(objHndl handle, feat_enums feat, bool checkPrereq = false); // note: checkPrereq only generates a warning if it's not met

	uint32_t(__cdecl *featTabLineParser)(TabFileStatus*, uint32_t, const char**);
	LegacyFeatSystem();
	BOOL FeatSystemInit();
	void FeatSystemShutdown();
protected:
	std::map<feat_enums, NewFeatSpec> mNewFeats;
	void _GetNewFeatsFromFile();
	void _GeneratePowerCriticalChildFeats(const NewFeatSpec &parentFeat);
	void _CompileParents();
	void _AddFeat(const NewFeatSpec &featSpec);
};

extern LegacyFeatSystem feats;


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
const char * _GetFeatDescription(feat_enums feat);
const char * _GetFeatPrereqDescription(feat_enums feat);
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