#pragma once
#include "python_integration.h"
#include "common.h"
#include <vector>
#include "d20_race.h"


class PythonRaceIntegration : public PythonIntegration {
public:

	enum class RaceSpecFunc : int {

		GetConditionName = 0, // gets the CondStruct's name (? id)

		GetRaceHelpTopic,

		GetRaceDefFlags,

		GetSkillPtsPerLevel,

		GetLevelModifier, // modifier for XP requirements

		GetFavoredClass,

		GetProtoId,

		GetMinMaxHeightWeight,

		GetStatModifiers,

		GetMaterialOffset,

	};

	PythonRaceIntegration();

	D20RaceSys::RaceDefinitionFlags GetRaceDefinitionFlags(int raceEnum);
	void GetRaceEnums(std::vector<int>& raceEnums);
	std::string GetConditionName(int raceEnum);
	std::string GetRaceHelpTopic(int raceEnum);
	std::vector<int> GetMinMaxHeightWeight(int raceEnum);
	std::vector<int> GetStatModifiers(Race race);
	int GetMaterialOffset(Race raceEnum);
	int GetProtoId(int raceEnum);

	int GetInt(int raceEnum, RaceSpecFunc specType, int defaultVal = 0);
	


protected:
	const char* GetFunctionName(EventId evt) override;
};

extern PythonRaceIntegration pythonRaceIntegration;
