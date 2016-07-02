#pragma once
#include "python_integration.h"
#include "common.h"

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
	GetConditionName, // gets the CondStruct's name (? id)


	IsAlignmentCompatible,
	ObjMeetsPreqreqs,
	GetFeat,
};


class PythonClassSpecIntegration : public PythonIntegration {
public:
	PythonClassSpecIntegration();

	void GetClassEnums(std::vector<int>& classEnums);
	std::string GetConditionName(int classEnum);
	int GetBabProgression(int classEnum);
	int GetHitDieType(int classEnum);
	int GetInt(int classEnum, ClassSpecFunc specType, int defaultVal = 0);
	bool IsSaveFavored(int classEnum, SavingThrowType saveType);
	SpellListType GetSpellListType(int classCode);
	bool IsEnabled(int classEnum);
	bool IsClassSkill(int classCode, int skillEnum);
	int IsClassFeat(int classCode, int featEnum);

	bool ReqsMet(const objHndl &handle, int classEnum);

protected:
	const char* GetFunctionName(EventId evt) override;
};

extern PythonClassSpecIntegration pythonClassIntegration;
