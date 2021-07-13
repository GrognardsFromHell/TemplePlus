#pragma once
#include "python_integration.h"
#include "common.h"

enum class ClassSpecFunc : int {

	GetHitDieType = 0,
	IsEnabled,
	GetConditionName, // gets the CondStruct's name (? id)

	GetClassDefFlags,
	GetClassHelpTopic,

	GetBabProgression,
	GetSkillPtsPerLevel,
	IsFortSaveFavored,
	IsRefSaveFavored,
	IsWillSaveFavored,

	GetSpellListType,
	GetSpellReadyingType,
	GetSpellSourceType,
	GetSpellList,
	GetSpellsPerDay,
	GetCasterLevels,
	GetSpellConditionName,
	GetSpellDeterminingStat,
	GetSpellDcStat,
	
	
	
	IsClassSkill,
	IsClassFeat,
	


	IsAlignmentCompatible,
	ObjMeetsPrereqs,
	GetDeityClass, // for the purposes of determining compatible deities.
	GetFeats,

	// levelup callbacks
	IsSelectingFeaturesOnLevelup,
	LevelupFeaturesInit,
	LevelupFeaturesCheck,
	LevelupFeaturesFinalize,

	IsSelectingFeatsOnLevelup,
	LevelupGetBonusFeats,

	LevelupCheckSpells,
	IsSelectingSpellsOnLevelup,
	LevelupInitSpellSelection,
	LevelupSpellsFinalize,
	GetAdvancedLearningClass,
	HasAdvancedLearning,
	HasArmoredArcaneCasterFeature,
};


class PythonClassSpecIntegration : public PythonIntegration {
public:
	PythonClassSpecIntegration();

	void GetClassEnums(std::vector<int>& classEnums);
	std::string GetConditionName(int classEnum);
	std::string GetClassHelpTopic(int classEnum);

	std::map<feat_enums, int> GetFeats(int classEnum);
	
	ClassDefinitionFlag GetClassDefinitionFlags(int classEnum);
	bool HasArmoredArcaneCasterFeature(int classEnum);
	int GetBabProgression(int classEnum);
	int GetHitDieType(int classEnum);
	int GetInt(int classEnum, ClassSpecFunc specType, int defaultVal = 0);
	bool IsSaveFavored(int classEnum, SavingThrowType saveType);
	
	SpellListType GetSpellListType(int classEnum);
	SpellReadyingType GetSpellReadyingType(int classEnum);
	SpellSourceType GetSpellSourceType(int classEnum);
	int GetAdvancedLearningClass(int classEnum);
	bool HasAdvancedLearning(int classEnum);
	std::map<int, int> GetSpellList(int classEnum); // returns a mapping of spellEnum -> spell level for this class, to be used by the spell system
	std::map<int, std::vector<int>> GetSpellsPerDay(int classEnum);
	std::string GetSpellCastingConditionName(int classEnum);
	Stat GetSpellDeterminingStat(int classEnum);
	Stat GetSpellDcStat(int classEnum);
	std::vector<int> GetCasterLevels(int classEnum);

	bool IsEnabled(int classEnum);
	bool IsClassSkill(int classCode, int skillEnum);
	int IsClassFeat(int classCode, int featEnum);

	bool IsAlignmentCompatible(const objHndl &handle, int classEnum); // checks if class is compatible with critter's alignment. Not relevant for character creation, only levelup (since alignment is determined after class selection)
	bool ReqsMet(const objHndl &handle, int classEnum);

	Stat GetDeityClass(int classEnum);

	// levelup
	bool IsSelectingFeaturesOnLevelup(objHndl handle, Stat classEnum);
	void LevelupFeaturesInit(objHndl handle, Stat classEnum, int classLvlNew = -1);
	bool LevelupFeaturesCheckComplete(objHndl handle, Stat classEnum, int classLvlNew = -1);
	void LevelupFeaturesFinalize(objHndl handle, Stat classEnum, int classLvlNew = -1);

	bool IsSelectingFeatsOnLevelup(objHndl handle, Stat classEnum);
	void LevelupGetBonusFeats(objHndl handle, Stat classEnum);

	bool IsSelectingSpellsOnLevelup(objHndl handle, Stat classEnum);
	void LevelupInitSpellSelection(objHndl handle, Stat classEnum, int classLvlNew = -1, int classLvlIncrease = 1);
	bool LevelupSpellsCheckComplete(objHndl handle, Stat classEnum);
	void LevelupSpellsFinalize(objHndl handle, Stat classEnum, int classLvlNew = -1);
	

protected:
	const char* GetFunctionName(EventId evt) override;
};

extern PythonClassSpecIntegration pythonClassIntegration;
