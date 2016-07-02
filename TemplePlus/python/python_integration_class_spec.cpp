#include "stdafx.h"
#include "python_integration_class_spec.h"
#include <gamesystems/gamesystems.h>
#include <gamesystems/objects/objsystem.h>
#include "python_object.h"

PythonClassSpecIntegration pythonClassIntegration;

PythonClassSpecIntegration::PythonClassSpecIntegration()
	:PythonIntegration("rules\\char_class\\Class*.py", "(class(\\d{3,}).*)\\.py"){
}

void PythonClassSpecIntegration::GetClassEnums(std::vector<int>& classEnums){

	for (auto it : mScripts){
		classEnums.push_back(it.first);
	}
}

std::string PythonClassSpecIntegration::GetConditionName(int classEnum){
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return fmt::format(""); 

	return RunScriptStringResult(classSpecEntry->second.id, (EventId)ClassSpecFunc::GetConditionName, nullptr);
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
		return RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::IsFortSaveFavored, nullptr) != 0;
		case SavingThrowType::Reflex:
			return RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::IsRefSaveFavored, nullptr) != 0;
		case SavingThrowType::Will:
			return RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::IsWillSaveFavored, nullptr) != 0;
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
	return GetInt(classEnum, ClassSpecFunc::IsEnabled) != 0;
}

bool PythonClassSpecIntegration::IsClassSkill(int classEnum, int skillEnum){
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return false;

	auto args = Py_BuildValue("(i)", skillEnum);
	auto result = RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::IsClassSkill, args) != 0; //classSpec->second
	Py_DECREF(args);
	return result;
}

int PythonClassSpecIntegration::IsClassFeat(int classCode, int featEnum)
{
	auto classSpecEntry = mScripts.find(classCode);
	if (classSpecEntry == mScripts.end())
		return false;

	auto args = Py_BuildValue("(i)", featEnum);
	auto result = RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::IsClassFeat, args) != 0; //classSpec->second
	Py_DECREF(args);
	return result;
}

bool PythonClassSpecIntegration::ReqsMet(const objHndl & handle, int classEnum){
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return false;
	
	// check alignment
	auto obj = gameSystems->GetObj().GetObject(handle);
	auto objAlignment = obj->GetInt32(obj_f_critter_alignment);
	auto args = Py_BuildValue("(i)", objAlignment);
	auto result = RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::IsAlignmentCompatible, args) != 0;
	Py_DECREF(args);
	if (!result)
		return false;

	auto attachee = PyObjHndl_Create(handle);
	args = Py_BuildValue("(O)", attachee);
	Py_DECREF(attachee);
	result = RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::ObjMeetsPreqreqs, args) != 0;
	Py_DECREF(args);
	return result;

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
	{ ClassSpecFunc::IsClassSkill,"IsClassSkill" },
	{ ClassSpecFunc::IsClassFeat,"IsClassFeat" },
	{ ClassSpecFunc::IsEnabled,"IsEnabled" },
	{ ClassSpecFunc::GetConditionName,"GetConditionName" },

	{ ClassSpecFunc::IsAlignmentCompatible,"IsAlignmentCompatible" },

	{ ClassSpecFunc::ObjMeetsPreqreqs,"ObjMeetsPreqreqs" },
	{ ClassSpecFunc::GetFeat,"GetFeat" }
};

const char* PythonClassSpecIntegration::GetFunctionName(EventId evt) {
	auto _evt = (ClassSpecFunc)evt;
	return classSpecFunctions[_evt].c_str();
}
