#pragma once
#include "python_integration.h"
#include "common.h"

enum class ClassSpecFunc : int {

	GetHitDieType = 0,
	GetClassDefFlags,
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
	GetSpellConditionName,
	GetSpellDeterminingStat,
	
	
	IsEnabled,
	IsClassSkill,
	IsClassFeat,
	GetConditionName, // gets the CondStruct's name (? id)


	IsAlignmentCompatible,
	ObjMeetsPrereqs,
	GetDeityClass, // for the purposes of determining compatible deities.
	GetFeats,

	// levelup callbacks
	IsSelectingFeatsOnLevelup,
	LevelupGetBonusFeats,

	LevelupCheckSpells,
	IsSelectingSpellsOnLevelup,
	LevelupInitSpellSelection,
	LevelupSpellsFinalize,
};


class PythonClassSpecIntegration : public PythonIntegration {
public:
	PythonClassSpecIntegration();

	void GetClassEnums(std::vector<int>& classEnums);
	std::string GetConditionName(int classEnum);
	std::map<feat_enums, int> GetFeats(int classEnum);
	
	ClassDefinitionFlag GetClassDefinitionFlags(int classEnum);
	int GetBabProgression(int classEnum);
	int GetHitDieType(int classEnum);
	int GetInt(int classEnum, ClassSpecFunc specType, int defaultVal = 0);
	bool IsSaveFavored(int classEnum, SavingThrowType saveType);
	
	SpellListType GetSpellListType(int classEnum);
	SpellReadyingType GetSpellReadyingType(int classEnum);
	SpellSourceType GetSpellSourceType(int classEnum);
	std::map<int, int> GetSpellList(int classEnum); // returns a mapping of spellEnum -> spell level for this class, to be used by the spell system
	std::map<int, std::vector<int>> GetSpellsPerDay(int classEnum);
	std::string GetSpellCastingConditionName(int classEnum);
	Stat GetSpellDeterminingStat(int classEnum);

	bool IsEnabled(int classEnum);
	bool IsClassSkill(int classCode, int skillEnum);
	int IsClassFeat(int classCode, int featEnum);

	bool ReqsMet(const objHndl &handle, int classEnum);

	Stat GetDeityClass(int classEnum);

	// levelup
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
