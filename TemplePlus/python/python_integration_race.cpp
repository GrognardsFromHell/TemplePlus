#include "stdafx.h"
#include "python_integration_race.h"
#include "d20_race.h"
#include "python_object.h"

PythonRaceIntegration pythonRaceIntegration;

PythonRaceIntegration::PythonRaceIntegration() 
	:PythonIntegration("rules\\races\\Race*.py", "(race(\\d{3,}).*)\\.py")
{

}


void PythonRaceIntegration::GetRaceEnums(std::vector<int>& raceEnums)
{
	for (auto it : mScripts) {
		raceEnums.push_back(it.first);
	}
}


int PythonRaceIntegration::GetInt(int raceEnum, RaceSpecFunc specType, int defaultVal)
{
	auto raceSpecEntry = mScripts.find(raceEnum);
	if (raceSpecEntry == mScripts.end())
		return defaultVal;

	return RunScriptDefault0(raceSpecEntry->second.id, (EventId)specType, nullptr);
}

int PythonRaceIntegration::GetLevelModifier(int raceEnum){
	return static_cast<int>(GetInt(raceEnum, RaceSpecFunc::GetLevelModifier, 0));
}

// ToEE already does a D20 query for favored class thankfully
//Stat PythonRaceIntegration::GetFavoredClass(objHndl handle){
//	auto race = GetRace(handle);
//	auto raceSpecEntry = mScripts.find(race);
//	if (raceSpecEntry == mScripts.end())
//		return stat_level_fighter;
//
//	auto attachee = PyObjHndl_Create(handle);
//	auto args = Py_BuildValue("(O)", attachee);
//	Py_DECREF(attachee);
//
//	auto result = RunScriptDefault0(raceSpecEntry->second.id, (EventId)RaceSpecFunc::GetFavoredClass, args);
//	Py_DECREF(args);
//
//	if (!result)
//		return stat_level_fighter;
//	
//	return (Stat)result;
//}


static std::map<PythonRaceIntegration::RaceSpecFunc, std::string> raceSpecFunctions = {
	// race spec fetchers
	{ PythonRaceIntegration::RaceSpecFunc::GetLevelModifier,"GetLevelModifier" },
};

const char * PythonRaceIntegration::GetFunctionName(EventId evt)
{
	auto _evt = (RaceSpecFunc)evt;
	return raceSpecFunctions[_evt].c_str();
}

Race PythonRaceIntegration::GetRace(objHndl handle){
	return critterSys.GetRace(handle, false);
}
