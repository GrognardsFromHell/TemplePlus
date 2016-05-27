#include "stdafx.h"
#include "python_integration_class_spec.h"

PythonClassSpecIntegration pythonClassIntegration;

PythonClassSpecIntegration::PythonClassSpecIntegration()
	:PythonIntegration("rules\\char_class\\Class*.py", "(class(\\d{3,}).*)\\.py"){
}

void PythonClassSpecIntegration::GetClassEnums(std::vector<int>& classEnums){

	for (auto it : mScripts){
		classEnums.push_back(it.first);
	}
}

int PythonClassSpecIntegration::GetBabProgression(int classEnum){
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return 1; // default to 3/4 type

	return RunScript(classSpecEntry->second.id, (int)ClassSpecFunc::GetBabProgression, nullptr); //classSpec->second
}

static std::map<ClassSpecFunc, std::string> classSpecFunctions = {
	// class spec fetchers
	{ClassSpecFunc::GetHitDieType,"GetHitDieType"},
	{ ClassSpecFunc::GetBabProgression,"GetBabProgression" },
	{ ClassSpecFunc::GetSkillPtsPerLevel,"GetSkillPtsPerLevel" },
	{ ClassSpecFunc::IsFortSaveFavored,"IsFortSaveFavored" },
	{ ClassSpecFunc::IsRefSaveFavored,"IsRefSaveFavored" },
	{ ClassSpecFunc::IsWillSaveFavored,"IsWillSaveFavored" },
	{ ClassSpecFunc::IsAlignmentCompatible,"IsAlignmentCompatible" },

	{ ClassSpecFunc::ObjMeetsPreqreqs,"ObjMeetsPreqreqs" },
	{ ClassSpecFunc::GetFeat,"GetFeat" }
};

const char* PythonClassSpecIntegration::GetFunctionName(EventId evt) {
	auto _evt = (ClassSpecFunc)evt;
	return classSpecFunctions[_evt].c_str();
}
