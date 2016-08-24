#include "stdafx.h"
#include "python_integration_d20_action.h"
#include <gamesystems/gamesystems.h>
#include <gamesystems/objects/objsystem.h>
#include "python_object.h"

PythonD20ActionIntegration pythonD20ActionIntegration;

PythonD20ActionIntegration::PythonD20ActionIntegration()
	:PythonIntegration("rules\\d20_actions\\action*.py", "(action(\\d{3,}).*)\\.py"){
}

void PythonD20ActionIntegration::GetActionEnums(std::vector<int>& actionEnums){

	for (auto it : mScripts){
		actionEnums.push_back(it.first);
	}
}

std::string PythonD20ActionIntegration::GetActionName(int actionEnum){
	auto actionSpecEntry = mScripts.find(actionEnum);
	if (actionSpecEntry == mScripts.end())
		return fmt::format(""); 

	return RunScriptStringResult(actionSpecEntry->second.id, (EventId)D20ActionSpecFunc::GetActionName, nullptr);
}



int PythonD20ActionIntegration::GetInt(int actionEnum, D20ActionSpecFunc specType, int defaultVal){
	auto actionSpecEntry = mScripts.find(actionEnum);
	if (actionSpecEntry == mScripts.end())
		return defaultVal; 

	return RunScript(actionSpecEntry->second.id, (EventId)specType, nullptr);
}

int PythonD20ActionIntegration::GetActionDefinitionFlags(int actionEnum){
	return GetInt(actionEnum, D20ActionSpecFunc::GetActionDefinitionFlags);
}

int PythonD20ActionIntegration::GetTargetingClassification(int actionEnum){
	return GetInt(actionEnum, D20ActionSpecFunc::GetTargetingClassification);
}


static std::map<D20ActionSpecFunc, std::string> D20ActionSpecFunctions = {

	{ D20ActionSpecFunc::GetActionDefinitionFlags,"GetActionDefinitionFlags"},
	{ D20ActionSpecFunc::GetActionName,"GetActionName" },
	{ D20ActionSpecFunc::GetTargetingClassification,"GetTargetingClassification" },
	
	
};

const char* PythonD20ActionIntegration::GetFunctionName(EventId evt) {
	auto _evt = (D20ActionSpecFunc)evt;
	return D20ActionSpecFunctions[_evt].c_str();
}
