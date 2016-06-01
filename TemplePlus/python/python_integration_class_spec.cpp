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

	return RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::GetBabProgression, nullptr); //classSpec->second
}

int PythonClassSpecIntegration::GetHitDieType(int classEnum){
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return 6; // default to 1d6

	return RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::GetHitDieType, nullptr); //classSpec->second
}

int PythonClassSpecIntegration::GetInt(int classEnum, ClassSpecFunc specType, int defaultVal){
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return defaultVal; 

	return RunScript(classSpecEntry->second.id, (EventId)specType, nullptr); //classSpec->second
}

bool PythonClassSpecIntegration::IsSaveFavored(int classEnum, SavingThrowType saveType){
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return 1; // default to 3/4 type
	switch (saveType){
	case SavingThrowType::Fortitude:
		return RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::IsFortSaveFavored, nullptr);
		case SavingThrowType::Reflex:
			return RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::IsRefSaveFavored, nullptr);
		case SavingThrowType::Will:
			return RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::IsWillSaveFavored, nullptr);
		default:
			return false;
	}
}

SpellListType PythonClassSpecIntegration::GetSpellListType(int classEnum){
	return static_cast<SpellListType>(GetInt(classEnum, ClassSpecFunc::GetSpellListType, (int)SpellListType::None));
}

bool PythonClassSpecIntegration::IsEnabled(int classEnum){
	if (classEnum <= stat_level_wizard && classEnum >= stat_level_barbarian)
		return true; // vanilla classes
	return GetInt(classEnum, ClassSpecFunc::IsEnabled);
}

static std::map<ClassSpecFunc, std::string> classSpecFunctions = {
	// class spec fetchers
	{ClassSpecFunc::GetHitDieType,"GetHitDieType"},
	{ ClassSpecFunc::GetBabProgression,"GetBabProgression" },
	{ ClassSpecFunc::GetSkillPtsPerLevel,"GetSkillPtsPerLevel" },
	{ ClassSpecFunc::IsFortSaveFavored,"IsFortSaveFavored" },
	{ ClassSpecFunc::IsRefSaveFavored,"IsRefSaveFavored" },
	{ ClassSpecFunc::IsWillSaveFavored,"IsWillSaveFavored" },
	{ ClassSpecFunc::GetSpellListType,"GetSpellListType" },
	{ ClassSpecFunc::IsEnabled,"IsEnabled" },

	{ ClassSpecFunc::IsAlignmentCompatible,"IsAlignmentCompatible" },

	{ ClassSpecFunc::ObjMeetsPreqreqs,"ObjMeetsPreqreqs" },
	{ ClassSpecFunc::GetFeat,"GetFeat" }
};

const char* PythonClassSpecIntegration::GetFunctionName(EventId evt) {
	auto _evt = (ClassSpecFunc)evt;
	return classSpecFunctions[_evt].c_str();
}
