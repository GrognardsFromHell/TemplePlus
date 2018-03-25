#include "stdafx.h"
#include "python_integration_race.h"
#include "d20_race.h"

PythonRaceIntegration pythonRaceIntegration;

PythonRaceIntegration::PythonRaceIntegration() 
	:PythonIntegration("rules\\races\\Race*.py", "(race(\\d{3,}).*)\\.py")
{

}

D20RaceSys::RaceDefinitionFlags PythonRaceIntegration::GetRaceDefinitionFlags(int raceEnum)
{
	auto raceSpecEntry = mScripts.find(raceEnum);
	if (raceSpecEntry == mScripts.end())
		return D20RaceSys::RaceDefinitionFlags::RDF_Vanilla;

	return (D20RaceSys::RaceDefinitionFlags)RunScript(raceSpecEntry->second.id, (EventId)RaceSpecFunc::GetRaceDefFlags, nullptr);
}

void PythonRaceIntegration::GetRaceEnums(std::vector<int>& raceEnums)
{
	for (auto it : mScripts) {
		raceEnums.push_back(it.first);
	}
}

std::string PythonRaceIntegration::GetConditionName(int raceEnum)
{
	auto raceSpecEntry = mScripts.find(raceEnum);
	if (raceSpecEntry == mScripts.end())
		return fmt::format("");

	return RunScriptStringResult(raceSpecEntry->second.id, (EventId)RaceSpecFunc::GetConditionName, nullptr);
}

std::string PythonRaceIntegration::GetRaceHelpTopic(int raceEnum)
{
	auto raceSpecEntry = mScripts.find(raceEnum);
	if (raceSpecEntry == mScripts.end())
		return fmt::format("");

	return RunScriptStringResult(raceSpecEntry->second.id, (EventId)RaceSpecFunc::GetRaceHelpTopic, nullptr);
}

std::vector<int> PythonRaceIntegration::GetMinMaxHeightWeight(int raceEnum)
{
	std::vector<int> result;
	auto raceSpecEntry = mScripts.find(raceEnum);
	if (raceSpecEntry == mScripts.end())
		return result;
	result = RunScriptVectorResult(raceSpecEntry->second.id, (EventId)RaceSpecFunc::GetMinMaxHeightWeight, nullptr);
	return result;
}

std::vector<int> PythonRaceIntegration::GetStatModifiers(Race raceEnum){

	std::vector<int> result = {0,0,0,  0,0,0};
	auto raceSpecEntry = mScripts.find(raceEnum);
	if (raceSpecEntry == mScripts.end())
		return result;
	result = RunScriptVectorResult(raceSpecEntry->second.id, (EventId)RaceSpecFunc::GetStatModifiers, nullptr);

	// failsafe - ensure size 6 vector
	for (auto i= result.size(); i < 6; i++){
		result.push_back(0);
	}
	return result;
}

int PythonRaceIntegration::GetMaterialOffset(Race raceEnum){
	return static_cast<int>(GetInt(raceEnum, RaceSpecFunc::GetMaterialOffset, 0));
}

int PythonRaceIntegration::GetProtoId(int raceEnum) {
	return static_cast<int>(GetInt(raceEnum, RaceSpecFunc::GetProtoId, 13000));
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

static std::map<PythonRaceIntegration::RaceSpecFunc, std::string> raceSpecFunctions = {
	// race spec fetchers
	{ PythonRaceIntegration::RaceSpecFunc::GetConditionName,"GetConditionName" },
	{ PythonRaceIntegration::RaceSpecFunc::GetFavoredClass,"GetFavoredClass" },
	{ PythonRaceIntegration::RaceSpecFunc::GetLevelModifier,"GetLevelModifier" },
	{ PythonRaceIntegration::RaceSpecFunc::GetProtoId,"GetProtoId" },
	{ PythonRaceIntegration::RaceSpecFunc::GetRaceDefFlags,"GetRaceDefFlags" },
	{ PythonRaceIntegration::RaceSpecFunc::GetRaceHelpTopic,"GetRaceHelpTopic" },
	{ PythonRaceIntegration::RaceSpecFunc::GetMinMaxHeightWeight,"GetMinMaxHeightWeight" },
	{ PythonRaceIntegration::RaceSpecFunc::GetMaterialOffset,"GetMaterialOffset" },
	{ PythonRaceIntegration::RaceSpecFunc::GetStatModifiers,"GetStatModifiers" },
};

const char * PythonRaceIntegration::GetFunctionName(EventId evt)
{
	auto _evt = (RaceSpecFunc)evt;
	return raceSpecFunctions[_evt].c_str();
}
