#pragma once
#include "python_integration.h"


enum class ClassSpecFunc : int {

	GetHitDieType = 0,
	GetBabProgression,
	GetSkillPtsPerLevel,
	IsFortSaveFavored,
	IsRefSaveFavored,
	IsWillSaveFavored,
	IsAlignmentCompatible,

	ObjMeetsPreqreqs,
	GetFeat,
};


class PythonClassSpecIntegration : public PythonIntegration {
public:
	PythonClassSpecIntegration();

	void GetClassEnums(std::vector<int>& classEnums);
	int GetBabProgression(int classEnum);

protected:
	const char* GetFunctionName(EventId evt) override;
};

extern PythonClassSpecIntegration pythonClassIntegration;
