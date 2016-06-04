#pragma once
#include "python_integration.h"


enum class ClassSpecFunc : int {

	GetHitDieType = 0,
	GetBabProgression,
	GetSkillPtsPerLevel,
	IsFortSaveFavored,
	IsRefSaveFavored,
	IsWillSaveFavored,
	GetSpellListType,
	IsEnabled,
	IsClassSkill,
	IsClassFeat,


	IsAlignmentCompatible,
	ObjMeetsPreqreqs,
	GetFeat,
};


class PythonClassSpecIntegration : public PythonIntegration {
public:
	PythonClassSpecIntegration();

	void GetClassEnums(std::vector<int>& classEnums);
	int GetBabProgression(int classEnum);
	int GetHitDieType(int classEnum);
	int GetInt(int classEnum, ClassSpecFunc specType, int defaultVal = 0);
	bool IsSaveFavored(int classEnum, SavingThrowType saveType);
	SpellListType GetSpellListType(int classCode);
	bool IsEnabled(int classEnum);
	bool IsClassSkill(int classCode, int skillEnum);
	int IsClassFeat(int classCode, int featEnum);
protected:
	const char* GetFunctionName(EventId evt) override;
};

extern PythonClassSpecIntegration pythonClassIntegration;
