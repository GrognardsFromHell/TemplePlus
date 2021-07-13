#include "stdafx.h"
#include "python_integration_class_spec.h"
#include <gamesystems/gamesystems.h>
#include <gamesystems/objects/objsystem.h>
#include "python_object.h"
#include <config/config.h>
#include <pybind11/embed.h>

namespace py = pybind11;

PythonClassSpecIntegration pythonClassIntegration;


PYBIND11_EMBEDDED_MODULE(d20class, m) {
	m.doc() = "D20 Class module, used for D20 Class specs.";

	m.def("get_spell_stat", [](int classEnum)->int {
		auto classStat = (Stat)classEnum;
		return (int)d20ClassSys.GetSpellStat(classStat);
	});
}

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

std::string PythonClassSpecIntegration::GetClassHelpTopic(int classEnum)
{
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return fmt::format("");

	return RunScriptStringResult(classSpecEntry->second.id, (EventId)ClassSpecFunc::GetClassHelpTopic, nullptr);
}

std::map<feat_enums, int> PythonClassSpecIntegration::GetFeats(int classEnum)
{
	auto result =  std::map<feat_enums, int>();
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return result;

	auto dict = RunScriptMapResult(classSpecEntry->second.id, (EventId)ClassSpecFunc::GetFeats, nullptr);

	for (auto item: dict){
		for (auto vecItem: item.second)
		result[(feat_enums)vecItem] = item.first;
	}

	return result;
}

std::map<int, std::vector<int>> PythonClassSpecIntegration::GetSpellsPerDay(int classEnum)
{
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return std::map<int, std::vector<int>>();

	auto result = RunScriptMapResult(classSpecEntry->second.id, (EventId)ClassSpecFunc::GetSpellsPerDay, nullptr);


	return result;
}

std::string PythonClassSpecIntegration::GetSpellCastingConditionName(int classEnum)
{
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return fmt::format("");

	return RunScriptStringResult(classSpecEntry->second.id, (EventId)ClassSpecFunc::GetSpellConditionName, nullptr);
}

Stat PythonClassSpecIntegration::GetSpellDeterminingStat(int classEnum){

	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return stat_wisdom;
	return  (Stat)RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::GetSpellDeterminingStat, nullptr);
}

Stat PythonClassSpecIntegration::GetSpellDcStat(int classEnum)
{
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return stat_wisdom;
	return  (Stat)RunScriptDefault0(classSpecEntry->second.id, (EventId)ClassSpecFunc::GetSpellDcStat, nullptr);
}

std::vector<int> PythonClassSpecIntegration::GetCasterLevels(int classEnum)
{
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return std::vector<int>();

	auto result = RunScriptVectorResult(classSpecEntry->second.id, (EventId)ClassSpecFunc::GetCasterLevels, nullptr);

	return result;
}

ClassDefinitionFlag PythonClassSpecIntegration::GetClassDefinitionFlags(int classEnum){
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return ClassDefinitionFlag::CDF_BaseClass;

	return (ClassDefinitionFlag)RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::GetClassDefFlags, nullptr);
}

bool PythonClassSpecIntegration::HasArmoredArcaneCasterFeature(int classEnum) {
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return false; // default to false

	int res = GetInt(classSpecEntry->second.id, ClassSpecFunc::HasArmoredArcaneCasterFeature, 0);
	return res ? true : false;
}

int PythonClassSpecIntegration::GetBabProgression(int classEnum){
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return 1; // default to 3/4 type

	return RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::GetBabProgression, nullptr); 
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

	return RunScriptDefault0(classSpecEntry->second.id, (EventId)specType, nullptr); //classSpec->second
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

SpellReadyingType PythonClassSpecIntegration::GetSpellReadyingType(int classEnum)
{
	return static_cast<SpellReadyingType>(GetInt(classEnum, ClassSpecFunc::GetSpellReadyingType, (int)SpellReadyingType::Vancian));
}

SpellSourceType PythonClassSpecIntegration::GetSpellSourceType(int classEnum)
{
	return static_cast<SpellSourceType>(GetInt(classEnum, ClassSpecFunc::GetSpellSourceType, (int)SpellSourceType::Ability));
}

int PythonClassSpecIntegration::GetAdvancedLearningClass(int classEnum)
{
	return static_cast<enum Stat>(GetInt(classEnum, ClassSpecFunc::GetAdvancedLearningClass, classEnum));
}

bool PythonClassSpecIntegration::HasAdvancedLearning(int classEnum)
{
	return (GetInt(classEnum, ClassSpecFunc::GetAdvancedLearningClass, 0) != 0);
}

std::map<int, int> PythonClassSpecIntegration::GetSpellList(int classEnum)
{
	auto result = std::map<int, int>();
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return result;

	auto dict = RunScriptMapResult(classSpecEntry->second.id, (EventId)ClassSpecFunc::GetSpellList, nullptr);

	for (auto item : dict) {
		for (auto spEnum : item.second)
			result[spEnum] = item.first;
	}

	return result;
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

bool PythonClassSpecIntegration::IsAlignmentCompatible(const objHndl & handle, int classEnum)
{
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return false;

	if (config.laxRules && config.disableAlignmentRestrictions)
		return true;

	auto obj = gameSystems->GetObj().GetObject(handle);

	
	auto objAlignment = obj->GetInt32(obj_f_critter_alignment);
	auto args = Py_BuildValue("(i)", objAlignment);
	auto result = RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::IsAlignmentCompatible, args) != 0;
	Py_DECREF(args);
	
	return result;

}

bool PythonClassSpecIntegration::ReqsMet(const objHndl & handle, int classEnum){
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return false;
	
	auto obj = gameSystems->GetObj().GetObject(handle);
	
	
	auto attachee = PyObjHndl_Create(handle);
	auto args = Py_BuildValue("(O)", attachee);
	Py_DECREF(attachee);
	auto result = RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::ObjMeetsPrereqs, args) != 0;
	Py_DECREF(args);
	return result;
	
}

Stat PythonClassSpecIntegration::GetDeityClass(int classEnum){
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return (Stat)0;
	
	return (Stat)GetInt(classEnum, ClassSpecFunc::GetDeityClass, 0);
}

bool PythonClassSpecIntegration::LevelupSpellsCheckComplete(objHndl handle, Stat classEnum){
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return false;

	auto obj = gameSystems->GetObj().GetObject(handle);
	auto attachee = PyObjHndl_Create(handle);
	auto args = Py_BuildValue("(O)", attachee );
	Py_DECREF(attachee);

	auto result = RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::LevelupCheckSpells, args) != 0;
	Py_DECREF(args);
	return result != 0;
}



bool PythonClassSpecIntegration::IsSelectingFeaturesOnLevelup(objHndl handle, Stat classEnum)
{
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return false;

	auto attachee = PyObjHndl_Create(handle);
	auto args = Py_BuildValue("(O)", attachee);
	Py_DECREF(attachee);

	auto result = RunScriptDefault0(classSpecEntry->second.id, (EventId)ClassSpecFunc::IsSelectingFeaturesOnLevelup, args) != 0;
	Py_DECREF(args);
	return result;
}

void PythonClassSpecIntegration::LevelupFeaturesInit(objHndl handle, Stat classEnum, int classLvlNew)
{
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return;

	auto attachee = PyObjHndl_Create(handle);
	auto args = Py_BuildValue("(Oi)", attachee, classLvlNew);
	Py_DECREF(attachee);

	RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::LevelupFeaturesInit, args);
	Py_DECREF(args);
}

bool PythonClassSpecIntegration::LevelupFeaturesCheckComplete(objHndl handle, Stat classEnum, int classLvlNew)
{
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return false;

	auto obj = gameSystems->GetObj().GetObject(handle);
	auto attachee = PyObjHndl_Create(handle);
	auto args = Py_BuildValue("(Oi)", attachee, classLvlNew);
	Py_DECREF(attachee);

	auto result = RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::LevelupFeaturesCheck, args) != 0;
	Py_DECREF(args);
	return result != 0;
}

void PythonClassSpecIntegration::LevelupFeaturesFinalize(objHndl handle, Stat classEnum, int classLvlNew)
{
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return;

	auto attachee = PyObjHndl_Create(handle);
	auto args = Py_BuildValue("(Oi)", attachee, classLvlNew);
	Py_DECREF(attachee);

	RunScript(classSpecEntry->second.id, (EventId)ClassSpecFunc::LevelupFeaturesFinalize, args);
	Py_DECREF(args);
}

bool PythonClassSpecIntegration::IsSelectingFeatsOnLevelup(objHndl handle, Stat classEnum)
{
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return false;

	auto attachee = PyObjHndl_Create(handle);
	auto args = Py_BuildValue("(O)", attachee);
	Py_DECREF(attachee);

	auto result = RunScriptDefault0(classSpecEntry->second.id, (EventId)ClassSpecFunc::IsSelectingFeatsOnLevelup, args) != 0;
	Py_DECREF(args);
	return result;

}

void PythonClassSpecIntegration::LevelupGetBonusFeats(objHndl handle, Stat classEnum){
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return;

	auto attachee = PyObjHndl_Create(handle);
	auto args = Py_BuildValue("(O)", attachee);
	Py_DECREF(attachee);

	RunScriptDefault0(classSpecEntry->second.id, (EventId)ClassSpecFunc::LevelupGetBonusFeats, args) ;
	Py_DECREF(args);
}

bool PythonClassSpecIntegration::IsSelectingSpellsOnLevelup(objHndl handle, Stat classEnum){
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return false;

	auto attachee = PyObjHndl_Create(handle);
	auto args = Py_BuildValue("(O)", attachee);
	Py_DECREF(attachee);

	auto result = RunScriptDefault0(classSpecEntry->second.id, (EventId)ClassSpecFunc::IsSelectingSpellsOnLevelup, args) != 0;
	Py_DECREF(args);
	return result;
}
void PythonClassSpecIntegration::LevelupInitSpellSelection(objHndl handle, Stat classEnum, int classLvlNew , int classLvlIncrease)
{
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return;

	auto attachee = PyObjHndl_Create(handle);

	auto args = Py_BuildValue("(Oi)", attachee, classLvlNew);
	Py_DECREF(attachee);

	RunScriptDefault0(classSpecEntry->second.id, (EventId)ClassSpecFunc::LevelupInitSpellSelection, args) ;
	Py_DECREF(args);

}

void PythonClassSpecIntegration::LevelupSpellsFinalize(objHndl handle, Stat classEnum, int classLvlNew) {
	auto classSpecEntry = mScripts.find(classEnum);
	if (classSpecEntry == mScripts.end())
		return;

	auto attachee = PyObjHndl_Create(handle);
	auto args = Py_BuildValue("(Oi)", attachee, classLvlNew);
	Py_DECREF(attachee);

	RunScriptDefault0(classSpecEntry->second.id, (EventId)ClassSpecFunc::LevelupSpellsFinalize, args);
	Py_DECREF(args);
}

static std::map<ClassSpecFunc, std::string> classSpecFunctions = {
	// class spec fetchers
	{ClassSpecFunc::GetHitDieType,"GetHitDieType"},
	{ ClassSpecFunc::GetClassHelpTopic,"GetClassHelpTopic" },

	{ ClassSpecFunc::GetClassDefFlags,"GetClassDefinitionFlags" },
	{ ClassSpecFunc::GetBabProgression,"GetBabProgression" },
	{ ClassSpecFunc::GetSkillPtsPerLevel,"GetSkillPtsPerLevel" },
	{ ClassSpecFunc::IsFortSaveFavored,"IsFortSaveFavored" },
	{ ClassSpecFunc::IsRefSaveFavored,"IsRefSaveFavored" },
	{ ClassSpecFunc::IsWillSaveFavored,"IsWillSaveFavored" },

	{ ClassSpecFunc::GetSpellListType,"GetSpellListType" },
	{ ClassSpecFunc::GetSpellReadyingType,"GetSpellReadyingType" },
	{ ClassSpecFunc::GetAdvancedLearningClass,"GetAdvancedLearningClass" },
	{ ClassSpecFunc::HasAdvancedLearning,"HasAdvancedLearning" },
	{ ClassSpecFunc::HasArmoredArcaneCasterFeature, "HasArmoredArcaneCasterFeature" },
	{ ClassSpecFunc::GetSpellSourceType,"GetSpellSourceType" },
	{ ClassSpecFunc::GetSpellList,"GetSpellList" },
	{ ClassSpecFunc::GetSpellsPerDay,"GetSpellsPerDay" },
	{ ClassSpecFunc::GetCasterLevels,"GetCasterLevels" },
	{ ClassSpecFunc::GetSpellConditionName,"GetSpellCasterConditionName" },
	{ ClassSpecFunc::GetSpellDeterminingStat,"GetSpellDeterminingStat" },
	{ ClassSpecFunc::GetSpellDcStat,"GetSpellDcStat" },

	{ ClassSpecFunc::IsClassSkill,"IsClassSkill" },
	{ ClassSpecFunc::IsClassFeat,"IsClassFeat" },
	{ ClassSpecFunc::IsEnabled,"IsEnabled" },
	{ ClassSpecFunc::GetConditionName,"GetConditionName" },

	{ ClassSpecFunc::IsAlignmentCompatible,"IsAlignmentCompatible" },
	{ ClassSpecFunc::GetDeityClass,"GetDeityClass" },

	{ ClassSpecFunc::ObjMeetsPrereqs,"ObjMeetsPrereqs" },
	{ ClassSpecFunc::GetFeats,"GetClassFeats" },

	{ ClassSpecFunc::IsSelectingFeaturesOnLevelup, "IsSelectingFeaturesOnLevelup" },
	{ ClassSpecFunc::LevelupFeaturesInit, "LevelupFeaturesInit"},
	{ ClassSpecFunc::LevelupFeaturesFinalize, "LevelupFeaturesFinalize" },

	{ ClassSpecFunc::IsSelectingFeatsOnLevelup, "IsSelectingFeatsOnLevelup" },
	{ ClassSpecFunc::LevelupGetBonusFeats, "LevelupGetBonusFeats" },

	{ ClassSpecFunc::LevelupCheckSpells, "LevelupCheckSpells" },
	{ ClassSpecFunc::IsSelectingSpellsOnLevelup, "IsSelectingSpellsOnLevelup" },
	{ ClassSpecFunc::LevelupInitSpellSelection, "InitSpellSelection"},
	{ ClassSpecFunc::LevelupSpellsFinalize, "LevelupSpellsFinalize" },
};

const char* PythonClassSpecIntegration::GetFunctionName(EventId evt) {
	auto _evt = (ClassSpecFunc)evt;
	return classSpecFunctions[_evt].c_str();
}
