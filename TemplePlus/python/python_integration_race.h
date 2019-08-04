#pragma once
#include "python_integration.h"
#include "common.h"
#include <vector>
#include "d20_race.h"


class PythonRaceIntegration : public PythonIntegration {
public:

	enum class RaceSpecFunc : int {

		GetLevelModifier, // modifier for XP requirements

	};

	PythonRaceIntegration();
	void GetRaceEnums(std::vector<int>& raceEnums);
	
	int GetInt(int raceEnum, RaceSpecFunc specType, int defaultVal = 0);
	int GetLevelModifier(int raceEnum);

protected:
	const char* GetFunctionName(EventId evt) override;
	Race GetRace(objHndl handle);
};

extern PythonRaceIntegration pythonRaceIntegration;
