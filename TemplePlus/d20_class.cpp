#include "stdafx.h"
#include "d20_class.h"
#include "d20_level.h"
#include "obj.h"
#include <critter.h>
#include "python/python_integration_class_spec.h"
#include "util/fixes.h"
#include "gamesystems/d20/d20stats.h"
#include "gamesystems/legacysystems.h"

D20ClassSystem d20ClassSys;

struct D20ClassSystemAddresses : temple::AddressTable
{
	void(__cdecl *ClassPacketAlloc)(ClassPacket *classPkt); // allocates the three IdxTables within
	void(__cdecl *ClassPacketDealloc)(ClassPacket *classPkt);
	uint32_t(__cdecl * GetClassPacket)(Stat classEnum, ClassPacket* classPkt); // fills the struct with content based on classEnum (e.g. Barbarian Feats in the featsIdxTable). Also STUB FOR PRESTIGE CLASSES! TODO
	D20ClassSystemAddresses(){
		rebase(ClassPacketAlloc,   0x100F5730); 
		rebase(ClassPacketDealloc, 0x100F5780);
		rebase(GetClassPacket,     0x100F65E0);  
	}

} addresses;


class D20ClassHooks : public TempleFix
{
	void apply() override {

		// GetClassHD
		replaceFunction<int(Stat)>(0x10073C90, [](Stat classEnum)->int {
			return d20ClassSys.GetClassHitDice(classEnum);
		});

		// IsNonClassSkill
		replaceFunction<BOOL(SkillEnum, Stat classCode)>(0x1007D130, [](SkillEnum skillEnum, Stat classEnum)->BOOL
		{
			return !d20ClassSys.IsClassSkill(skillEnum, classEnum);
		});

		// UnableToLearnSkill
		replaceFunction<BOOL(SkillEnum, Stat classCode)>(0x1007D160, [](SkillEnum skillEnum, Stat classEnum)->BOOL
		{
			return !skillSys.IsEnabled(skillEnum);
			
			/* vanilla code allows learning a disabled skill if the class is proficient with it
			if (skillSys.IsEnabled(skillEnum))
				return 0;
			return !d20ClassSys.IsClassSkill(skillEnum, classEnum);*/
		});

		// IsNonClassSkillRegardDomain
		replaceFunction<BOOL(objHndl, SkillEnum, Stat)>(0x1007D4E0, [](objHndl obj, SkillEnum skillEnum, Stat classEnum)->BOOL
		{
			if (classEnum == stat_level_cleric)	{
				auto isDomainSkill = temple::GetRef<BOOL(__cdecl)(objHndl, SkillEnum)>(0x1004C0D0);
				if (isDomainSkill(obj, skillEnum))
					return FALSE;
			}
			return !d20ClassSys.IsClassSkill(skillEnum, classEnum);
		});

		// GetNumSkillPointsPerLevel
		replaceFunction<BOOL(int, Race, Stat)>(0x1007D3D0, [](int intScore, Race raceEnum, Stat classEnum)->BOOL
		{
			auto result = objects.GetModFromStatLevel(intScore);
			if (raceEnum == race_human) // todo: generalize with a dispatch
				result++;
			result += d20ClassSys.GetSkillPts(classEnum);
			return result;
		});

		// GetNumSpellsPerDayFromClass
		replaceFunction<int(objHndl, Stat, int , int )>(0x100F5660, [](objHndl handle, Stat classCode, int spellLvl, int classLvl)
		{
			return d20ClassSys.GetNumSpellsFromClass(handle, classCode, spellLvl, classLvl, true);
		});

	}
} d20ClassHooks;

bool D20ClassSystem::ReqsMet(const objHndl& handle, const Stat classCode){
	auto classSpec = classSpecs.find(classCode);
	if (classSpec == classSpecs.end())
		return false;

	return pythonClassIntegration.ReqsMet(handle, classCode);
}

bool D20ClassSystem::IsNaturalCastingClass(Stat classEnum, objHndl handle)
{
	if (classEnum == stat_level_bard || classEnum == stat_level_sorcerer) 
		return 1;
	return 0;
}

bool D20ClassSystem::IsNaturalCastingClass(uint32_t classEnum){
	return IsNaturalCastingClass((Stat)classEnum);
}

bool D20ClassSystem::IsVancianCastingClass(Stat classEnum, objHndl handle )
{
	if (classEnum == stat_level_cleric
		|| classEnum == stat_level_druid 
		|| classEnum == stat_level_paladin
		|| classEnum == stat_level_ranger
		|| classEnum == stat_level_wizard)
		return 1;
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return 0;
	
	return 0;
}

bool D20ClassSystem::IsCastingClass(Stat classEnum){
	/*if (classEnum == stat_level_cleric 
		|| classEnum == stat_level_druid 
		|| classEnum == stat_level_paladin 
		|| classEnum == stat_level_ranger 
		|| classEnum == stat_level_wizard
		|| classEnum == stat_level_bard
		|| classEnum == stat_level_sorcerer) return 1;*/
	
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return 0;
	if (classSpec->second.spellListType == SpellListType::None){
		return 0;
	}

	return 1;
}

bool D20ClassSystem::IsLateCastingClass(Stat classEnum)
{
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return false;

	if (classSpec->second.spellListType == SpellListType::Paladin
		|| classSpec->second.spellListType == SpellListType::Ranger)
		return true;

	auto &spellsPerDay = classSpec->second.spellsPerDay;
	auto spellsAtLvl1Spec = spellsPerDay.find(1);
	if (spellsAtLvl1Spec == spellsPerDay.end())
		return false; // no spells specified for class level 1

	for (auto it: spellsAtLvl1Spec->second){
		if (it != -1)
			return false;
	}
	
	return true;
}

bool D20ClassSystem::IsArcaneCastingClass(Stat classCode, objHndl handle){
	auto classSpec = classSpecs.find(classCode);
	if (classSpec == classSpecs.end())
		return false;

	if (classSpec->second.spellListType == SpellListType::Arcane
		|| classSpec->second.spellListType == SpellListType::Bardic)
		return true;

	// todo handle PrC that can be either

	return false;
}

bool D20ClassSystem::HasDomainSpells(Stat classEnum){
	if (classEnum == stat_level_cleric)
		return true;
	return false;
}

Stat D20ClassSystem::GetSpellStat(Stat classEnum){
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return stat_wisdom; 
	
	return classSpec->second.spellStat;
}

int D20ClassSystem::GetMaxSpellLevel(Stat classEnum, int characterLvl)
{
	auto result = -1;

	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return -1;


	auto &spellsPerDay = classSpec->second.spellsPerDay;

	auto spellsPerDayForLvl = spellsPerDay.find(characterLvl);
	
	// if not found, get highest specified
	if (spellsPerDayForLvl == spellsPerDay.end()){
		auto highestSpec = -1;
		for (auto it: spellsPerDay){
			if (it.first > highestSpec && it.first <= characterLvl)
				highestSpec = it.first;
		}
		if (highestSpec == -1)
			return -1;
		spellsPerDayForLvl = spellsPerDay.find(highestSpec);
	}
	
	
	
	auto &spellsVector = spellsPerDayForLvl->second;
	if (!spellsVector.size())
		return -1;
	if (spellsVector[spellsVector.size() - 1] == -1)
		return -1;

	return spellsVector.size() - 1; // first slot corresponds to level 0 spells
}

std::string D20ClassSystem::GetSpellCastingCondition(Stat classEnum){
	auto result = std::string();

	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return result;

	result.append(fmt::format("{}", classSpec->second.spellCastingConditionName));
	return result;
}

void D20ClassSystem::ClassPacketAlloc(ClassPacket* classPkt)
{
	addresses.ClassPacketAlloc(classPkt);
}

void D20ClassSystem::ClassPacketDealloc(ClassPacket* classPkt)
{
	addresses.ClassPacketDealloc(classPkt);
}

uint32_t D20ClassSystem::GetClassPacket(Stat classEnum, ClassPacket* classPkt){
	return addresses.GetClassPacket(classEnum, classPkt);
}

int D20ClassSystem::GetBaseAttackBonus(Stat classCode, uint32_t classLvl){
	auto classSpec = classSpecs.find(classCode);
	if (classSpec == classSpecs.end())
		return 0;
	
	auto babProg = classSpec->second.babProgression;

	switch (babProg){
	case BABProgressionType::Martial: 
		return classLvl ;
	case BABProgressionType::SemiMartial: 
		return (3 * classLvl ) / 4;
	case BABProgressionType::NonMartial: 
		return classLvl/ 2;
	default: 
		break;
	}
	logger->warn("D20ClassSys: GetBaseAttackBonus unhandled BAB progression");
	return 0;
}

bool D20ClassSystem::IsSaveFavoredForClass(Stat classCode, int saveType){
	auto classSpec = classSpecs.find(classCode);
	if (classSpec == classSpecs.end())
		return 0;
	switch (saveType)
	{
	case D20_Save_Fortitude:
		return classSpec->second.fortitudeSaveIsFavored;
	case D20_Save_Reflex:
		return classSpec->second.reflexSaveIsFavored;
	case D20_Save_Will:
		return classSpec->second.willSaveIsFavored;
	default:
		logger->warn("D20ClassSys: IsSaveFavoredForClass unhandled save type {}", saveType);
		break;
	}
	return false;
}

int D20ClassSystem::GetSkillPts(Stat classEnum){
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return 2;

	return classSpec->second.skillPts;
}

int D20ClassSystem::GetClassHitDice(Stat classEnum){
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return 6;

	return classSpec->second.hitDice;
}

const char* D20ClassSystem::GetClassShortHelp(Stat classCode){
	return d20Stats.GetClassShortDesc(classCode);
}

// D20ClassSpec
void D20ClassSystem::GetClassSpecs(){
	std::vector<int> _classEnums;
	pythonClassIntegration.GetClassEnums( _classEnums);
	std::sort(_classEnums.begin(), _classEnums.end());

	for (auto it : _classEnums){

		if (!pythonClassIntegration.IsEnabled(it))
			continue;

		classEnums.push_back(it);

		auto &classSpec = classSpecs[it];

		classSpec.classEnum = static_cast<Stat>(it);

		// get class condition from py file
		classSpec.conditionName = fmt::format("{}", pythonClassIntegration.GetConditionName(it));
		if (classSpec.conditionName.size() == 0){ // if none specified, get it from the pre-defined table (applicable for vanilla classes only...)
			classSpec.conditionName = d20StatusSys.classCondMap[classSpec.classEnum];
		}
		else if (d20StatusSys.classCondMap.find(classSpec.classEnum) == d20StatusSys.classCondMap.end() ){ // if specified, update the d20Status mapping
			d20StatusSys.classCondMap[classSpec.classEnum] = classSpec.conditionName;
		}


		classSpec.babProgression = static_cast<BABProgressionType>(pythonClassIntegration.GetBabProgression(it));
		classSpec.hitDice = pythonClassIntegration.GetHitDieType(it);
		classSpec.fortitudeSaveIsFavored = pythonClassIntegration.IsSaveFavored(it, SavingThrowType::Fortitude);
		classSpec.reflexSaveIsFavored = pythonClassIntegration.IsSaveFavored(it, SavingThrowType::Reflex);
		classSpec.willSaveIsFavored = pythonClassIntegration.IsSaveFavored(it, SavingThrowType::Will);
		classSpec.skillPts = pythonClassIntegration.GetInt(it, ClassSpecFunc::GetSkillPtsPerLevel, 2);

		// spell casting
		classSpec.spellListType = pythonClassIntegration.GetSpellListType(it);
		if (classSpec.spellListType != SpellListType::None){
			classSpec.spellCastingConditionName = fmt::format("{}", pythonClassIntegration.GetSpellCastingConditionName(it));
			classSpec.spellsPerDay = pythonClassIntegration.GetSpellsPerDay(it);
			if (classSpec.spellsPerDay.size()){
				classSpec.spellStat = pythonClassIntegration.GetSpellDeterminingStat(it);
			}
		}
		
		// skills
		for (auto skillEnum = static_cast<int>(skill_appraise); skillEnum < skill_count; skillEnum++) {
			classSpec.classSkills[(SkillEnum)skillEnum] = pythonClassIntegration.IsClassSkill(it, (SkillEnum)skillEnum);
		}

		// feats
		classSpec.classFeats = pythonClassIntegration.GetFeats(it);
		auto test = 1;
	}

}

int D20ClassSystem::ClericMaxSpellLvl(uint32_t clericLvl) const
{
	int result = clericLvl % 2 + clericLvl / 2;
	if (result < 0)
		result = 0;
	return result;
}

int D20ClassSystem::NumDomainSpellsKnownFromClass(objHndl dude, Stat classCode)
{
	if (classCode != stat_level_cleric)
		return 0;
	auto clericLvl = critterSys.GetCasterLevelForClass(dude, stat_level_cleric);
	return ClericMaxSpellLvl(clericLvl) * 2;
}

int D20ClassSystem::GetNumSpellsFromClass(objHndl obj, Stat classEnum, int spellLvl, uint32_t classLvl, bool getFromStatMod)
{
	auto result = -1;

	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return -1;


	auto &spellsPerDay = classSpec->second.spellsPerDay;

	auto spellsPerDayForLvl = spellsPerDay.find(classLvl);

	// if not found, get highest specified
	if (spellsPerDayForLvl == spellsPerDay.end()) {
		auto highestSpec = -1;
		for (auto it : spellsPerDay) {
			if (it.first > highestSpec && it.first <= (int)classLvl)
				highestSpec = it.first;
		}
		if (highestSpec == -1)
			return -1;
		spellsPerDayForLvl = spellsPerDay.find(highestSpec);
	}



	auto &spellsVector = spellsPerDayForLvl->second;
	if ((int)spellsVector.size() < spellLvl+1)
		return -1;
	if (spellsVector[spellLvl] < 0)
		return -1;
	
	result = spellsVector[spellLvl];

	if (!getFromStatMod || spellLvl == 0)
		return result;

	auto spellStat = GetSpellStat(classEnum);
	auto spellStatMod = objects.GetModFromStatLevel(objects.StatLevelGet(obj,spellStat));
	if (spellStatMod >= spellLvl)
		result += ( (spellStatMod - spellLvl) / 4) + 1;

	return result;

}

BOOL D20ClassSystem::IsClassSkill(SkillEnum skillEnum, Stat classCode){
	auto classSpec = classSpecs.find(classCode);
	if (classSpec == classSpecs.end())
		return FALSE;

	if (classSpec->second.classSkills[skillEnum])
		return TRUE;

	return FALSE;
}

bool D20ClassSystem::LevelupSpellsCheckComplete(objHndl handle, Stat classEnum){

	return pythonClassIntegration.LevelupSpellsCheckComplete(handle, classEnum);
}

void D20ClassSystem::LevelupSpellsFinalize(objHndl handle, Stat classEnum){
	if (objects.StatLevelGet(handle, classEnum))
		dispatch.DispatchLevelupSystemEvent(handle, classEnum, DK_LVL_Spells_Finalize);
	else
		pythonClassIntegration.LevelupSpellsFinalize(handle, classEnum);
	
}

bool D20ClassSystem::IsSelectingFeatsOnLevelup(objHndl handle, Stat classEnum){

	return pythonClassIntegration.IsSelectingFeatsOnLevelup(handle, classEnum);
}

bool D20ClassSystem::IsSelectingSpellsOnLevelup(objHndl handle, Stat classEnum){

	return pythonClassIntegration.IsSelectingSpellsOnLevelup(handle, classEnum);
}

void D20ClassSystem::LevelupInitSpellSelection(objHndl handle, Stat classEnum){
	if (objects.StatLevelGet(handle, classEnum))
		dispatch.DispatchLevelupSystemEvent(handle, classEnum, DK_LVL_Spells_Activate);
	else
		pythonClassIntegration.LevelupInitSpellSelection(handle, classEnum);
}
