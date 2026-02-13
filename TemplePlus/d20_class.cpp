#include "stdafx.h"
#include "d20_class.h"
#include "d20_level.h"
#include "obj.h"
#include <critter.h>
#include "python/python_integration_class_spec.h"
#include "util/fixes.h"
#include "gamesystems/d20/d20stats.h"
#include "gamesystems/legacysystems.h"
#include "gamesystems/deity/legacydeitysystem.h"
#include "ui/ui_char_editor.h"

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

	static int HookedLvl1SkillPts(int intMod);
	static int NumSkillPointsPerLevel(int intScore, Race rc, Stat cls);

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

				if (deitySys.IsDomainSkill(obj, skillEnum))
					return FALSE;
			}

			if (d20Sys.D20QueryPython(obj, "Is Class Skill", skillEnum)){
				return FALSE;
			}

			return !d20ClassSys.IsClassSkill(skillEnum, classEnum);
		});

		replaceFunction(0x1007D3D0, NumSkillPointsPerLevel);

		writeNoops(0x10181441);
		writeNoops(0x1018144E);
		writeCall(0x10181436, HookedLvl1SkillPts);

		// GetNumSpellsPerDayFromClass
		replaceFunction<int(objHndl, Stat, int , int )>(0x100F5660, [](objHndl handle, Stat classCode, int spellLvl, int classLvl)
		{
			return d20ClassSys.GetNumSpellsFromClass(handle, classCode, spellLvl, classLvl, true);
		});

	}
} d20ClassHooks;

bool D20ClassSystem::IsCoreClass(Stat classEnum){
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return false;

	return (classSpec->second.flags & CDF_CoreClass) != 0;
}

bool D20ClassSystem::IsBaseClass(Stat classEnum)
{
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return false;

	return (classSpec->second.flags & CDF_BaseClass) != 0;
}

bool D20ClassSystem::ReqsMet(const objHndl& handle, const Stat classCode){
	auto classSpec = classSpecs.find(classCode);
	if (classSpec == classSpecs.end())
		return false;

	return pythonClassIntegration.ReqsMet(handle, classCode);
}

bool D20ClassSystem::IsCompatibleWithAlignment(Stat classEnum, Alignment al){

	if (config.laxRules && config.disableAlignmentRestrictions)
		return true;

	return temple::GetRef<BOOL(__cdecl)(Stat, Alignment)>(0x10188170)(classEnum, al);
}

bool D20ClassSystem::IsNaturalCastingClass(Stat classEnum, objHndl handle){
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return false;
	
	return classSpec->second.spellMemorizationType == SpellReadyingType::Innate;	
}

bool D20ClassSystem::IsNaturalCastingClass(uint32_t classEnum){
	return IsNaturalCastingClass((Stat)classEnum);
}

bool D20ClassSystem::IsVancianCastingClass(Stat classEnum, objHndl handle )
{
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return false;

	return classSpec->second.spellMemorizationType == SpellReadyingType::Vancian;
}

bool D20ClassSystem::IsCastingClass(Stat classEnum, bool includeExtenders){
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return false;

	if (includeExtenders && classSpec->second.spellListType == SpellListType::Extender)
		return true;

	else if (classSpec->second.spellListType == SpellListType::None){
		return false;
	}

	return true;
}

bool D20ClassSystem::HasSpellList(Stat classEnum)
{
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return false;
	
	switch (classSpec->second.spellListType)
	{
		case SpellListType::None: 
			return false;
		case SpellListType::Arcane:
		case SpellListType::Bardic: 
		case SpellListType::Clerical: 
		case SpellListType::Druidic: 
		case SpellListType::Paladin: 
		case SpellListType::Ranger: 
		case SpellListType::Special: 
			return true;
		default: 
			return false;
	}
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

	if (classSpec->second.spellListType == SpellListType::None)
		return false;

	if (classSpec->second.spellSourceType == SpellSourceType::Arcane)
		return true;

	return false;
}

bool D20ClassSystem::IsDivineCastingClass(Stat classCode, objHndl handle)
{
	auto classSpec = classSpecs.find(classCode);
	if (classSpec == classSpecs.end())
		return false;

	if (classSpec->second.spellListType == SpellListType::None)
		return false;

	if (classSpec->second.spellSourceType == SpellSourceType::Divine)
		return true;

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

Stat D20ClassSystem::GetSpellDcStat(Stat classEnum){
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return stat_wisdom; 
	
	if (classSpec->second.spellDcStat == Stat::stat_strength)
		return classSpec->second.spellStat;

	return classSpec->second.spellDcStat;
}

Stat D20ClassSystem::GetDeityClass(Stat classEnum){
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return (Stat)0;

	auto deityClass = classSpec->second.deityClass;

	if (deityClass == (Stat)0)
		return classEnum;

	return deityClass;
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

int D20ClassSystem::GetCasterLevel(Stat classEnum, int classLvl){
	if (classLvl < 1)
		return -1;

	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return -1;
	
	std::vector<int> &casterLvl = classSpec->second.casterLvl;

	if (casterLvl.size() <= 0){
		return -1;
	}
	if (classLvl >= static_cast<int>(casterLvl.size())){
		return casterLvl[casterLvl.size() - 1];
	}
	
	return casterLvl[classLvl-1];
}

int D20ClassSystem::GetMinCasterLevelForSpellLevel(Stat classEnum, int spellLevel){
	auto result = -1;

	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return -1;

	auto &spellsPerDay = classSpec->second.spellsPerDay;

	auto minClassLvl = 999;
	for (auto it : spellsPerDay) {
		auto cl = it.first;
		if (cl > minClassLvl)
			continue;
		auto &spellsVector = it.second;
		int maxSpellLvl = (int)spellsVector.size() - 1;
		if (maxSpellLvl >= spellLevel && spellsVector[maxSpellLvl] >= 0){
			minClassLvl = it.first;
		}
	}
	if (minClassLvl == 999)
		return -1;
	return GetCasterLevel(classEnum, minClassLvl);
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

const std::vector<Stat>& D20ClassSystem::GetArmoredArcaneCasterFeatureClasses() {
	return armoredArcaneCasterFeatureClasses;
}

bool  D20ClassSystem::HasArmoredArcaneCasterFeature(Stat classCode){
	auto codeFound = std::find(armoredArcaneCasterFeatureClasses.begin(), armoredArcaneCasterFeatureClasses.end(), classCode);
	return codeFound != armoredArcaneCasterFeatureClasses.end();
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

int D20ClassSystem::GetClassEnum(const std::string& s){
	for (auto &classSpec:classSpecs){
		if (!_strcmpi(tolower(classSpec.second.conditionName).c_str(), tolower(s).c_str()))
			return classSpec.first;
	}
	return 0;
}

const char* D20ClassSystem::GetClassShortHelp(Stat classCode){
	return d20Stats.GetClassShortDesc(classCode);
}

std::string & D20ClassSystem::GetClassHelpTopic(Stat classEnum){
	static std::string noneString("");
	auto classSpec = classSpecs.find(classEnum);
	if (classSpec == classSpecs.end())
		return noneString;

	return classSpec->second.helpTopic;
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

		D20ClassSpec &classSpec = classSpecs[it];

		classSpec.classEnum = static_cast<Stat>(it);
		classSpec.helpTopic = pythonClassIntegration.GetClassHelpTopic(it);

		// get class condition from py file
		classSpec.conditionName = fmt::format("{}", pythonClassIntegration.GetConditionName(it));
		if (classSpec.conditionName.size() == 0){ // if none specified, get it from the pre-defined table (applicable for vanilla classes only...)
			classSpec.conditionName = d20StatusSys.classCondMap[classSpec.classEnum];
		}
		else if (d20StatusSys.classCondMap.find(classSpec.classEnum) == d20StatusSys.classCondMap.end() ){ // if specified, update the d20Status mapping
			d20StatusSys.classCondMap[classSpec.classEnum] = classSpec.conditionName;
		}

		classSpec.flags = pythonClassIntegration.GetClassDefinitionFlags(it);
		if (classSpec.flags & ClassDefinitionFlag::CDF_BaseClass)
			baseClassEnums.push_back(it);
		classSpec.babProgression = static_cast<BABProgressionType>(pythonClassIntegration.GetBabProgression(it));
		classSpec.hitDice = pythonClassIntegration.GetHitDieType(it);
		classSpec.fortitudeSaveIsFavored = pythonClassIntegration.IsSaveFavored(it, SavingThrowType::Fortitude);
		classSpec.reflexSaveIsFavored = pythonClassIntegration.IsSaveFavored(it, SavingThrowType::Reflex);
		classSpec.willSaveIsFavored = pythonClassIntegration.IsSaveFavored(it, SavingThrowType::Will);
		classSpec.skillPts = pythonClassIntegration.GetInt(it, ClassSpecFunc::GetSkillPtsPerLevel, 2);

		// spell casting
		classSpec.spellListType = pythonClassIntegration.GetSpellListType(it);
		classSpec.hasArmoredArcaneCasterFeature = pythonClassIntegration.HasArmoredArcaneCasterFeature(it);

		if (classSpec.hasArmoredArcaneCasterFeature) {
			armoredArcaneCasterFeatureClasses.push_back(static_cast<Stat>(it));
		}

		if (classSpec.spellListType != SpellListType::None){
			// Spell Readying Type
			classSpec.spellMemorizationType = pythonClassIntegration.GetSpellReadyingType(it);

			// Spell Source Type (Arcane/Divine/other...)
			classSpec.spellSourceType = pythonClassIntegration.GetSpellSourceType(it);

			if (pythonClassIntegration.HasAdvancedLearning(it)) {
				spellSys.RegisterAdvancedLearningClass(static_cast<Stat>(it));
			}

			// Spellcasting Condition
			classSpec.spellCastingConditionName = fmt::format("{}", pythonClassIntegration.GetSpellCastingConditionName(it));

			// Spells per day table
			classSpec.spellsPerDay = pythonClassIntegration.GetSpellsPerDay(it);
			if (classSpec.spellsPerDay.size()){
				classSpec.spellStat = pythonClassIntegration.GetSpellDeterminingStat(it);
				classSpec.spellDcStat = pythonClassIntegration.GetSpellDcStat(it);
				classSpec.casterLvl = pythonClassIntegration.GetCasterLevels(it);
			}

			static std::map<SpellListType, Stat> spellListMaps = {
				{ SpellListType::Arcane, stat_level_wizard },
				{ SpellListType::Bardic, stat_level_bard },
				{ SpellListType::Clerical, stat_level_cleric },
				{ SpellListType::Druidic, stat_level_druid },
				{ SpellListType::Paladin, stat_level_paladin },
				{ SpellListType::Ranger, stat_level_ranger },
			};

			auto findSpellListMap = spellListMaps.find(classSpec.spellListType);
			if (findSpellListMap != spellListMaps.end() && classSpec.classEnum != findSpellListMap->second && !(classSpec.flags & CDF_CoreClass)) {
				
				auto asdf = objHndl::null;
				std::vector<SpellEntry> entries;
				spellSys.CopyLearnableSpells(asdf, spellSys.GetSpellClass(findSpellListMap->second), entries);

				for (auto &sp:entries){
					auto lvl = sp.SpellLevelForSpellClass(spellSys.GetSpellClass(findSpellListMap->second));
					classSpec.spellList[sp.spellEnum] = lvl;
				}
				spellSys.GetSpellEntryExtFromClassSpec(classSpec.spellList, it);
			}

			if (classSpec.spellListType == SpellListType::Special){
				classSpec.spellList = pythonClassIntegration.GetSpellList(it);
				spellSys.GetSpellEntryExtFromClassSpec(classSpec.spellList, it);
			}

			if (HasSpellList((Stat)it))	{
				classEnumsWithSpellLists.push_back((Stat)it);
			}
		}

		// skills
		for (auto skillEnum = static_cast<int>(skill_appraise); skillEnum < skill_count; skillEnum++) {
			classSpec.classSkills[(SkillEnum)skillEnum] = pythonClassIntegration.IsClassSkill(it, (SkillEnum)skillEnum);
		}

		// feats
		classSpec.classFeats = pythonClassIntegration.GetFeats(it);
		auto test = 1;

		// deity
		classSpec.deityClass = pythonClassIntegration.GetDeityClass(it);
	}

}

/* 0x100F4BC0 */
int D20ClassSystem::ClericMaxSpellLvl(uint32_t clericLvl) const
{
	int result = clericLvl % 2 + clericLvl / 2;
	if (result < 0)
		result = 0;
	if (result > NUM_SPELL_LEVELS-1) // Temple+: fixes crash when cleric level > 20 in spell memo screen
		return NUM_SPELL_LEVELS - 1;
	return result;
}

int D20ClassSystem::NumDomainSpellsKnownFromClass(objHndl dude, Stat classCode)
{
	if (classCode != stat_level_cleric)
		return 0;
	auto clericLvl = critterSys.GetSpellListLevelForClass(dude, stat_level_cleric);
	return min( (NUM_SPELL_LEVELS-1)*2, ClericMaxSpellLvl(clericLvl) * 2);
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

bool D20ClassSystem::HasFeat(feat_enums featEnum, Stat classCode, int classLvl){
	auto classSpec = classSpecs.find(classCode);
	if (classSpec == classSpecs.end())
		return false;

	auto & classFeats = classSpec->second.classFeats;
	auto lvlSpec = classFeats.find(featEnum);
	if (lvlSpec == classFeats.end())
		return false;

	if (lvlSpec->second <= classLvl)
		return true;

	return false;
}

bool D20ClassSystem::LevelupSpellsCheckComplete(objHndl handle, Stat classEnum){
	if (objects.StatLevelGet(handle, classEnum)){
		auto result = dispatch.DispatchLevelupSystemEvent(handle, classEnum, DK_LVL_Spells_Check_Complete);
		return result >= 0;
	}
		
	else
		return pythonClassIntegration.LevelupSpellsCheckComplete(handle, classEnum);
}

void D20ClassSystem::LevelupSpellsFinalize(objHndl handle, Stat classEnum, int classLvlNew){
	if (objects.StatLevelGet(handle, classEnum) && classLvlNew == -1)
		dispatch.DispatchLevelupSystemEvent(handle, classEnum, DK_LVL_Spells_Finalize);
	else
		pythonClassIntegration.LevelupSpellsFinalize(handle, classEnum, classLvlNew);
	
}

bool D20ClassSystem::IsSelectingFeatsOnLevelup(objHndl handle, Stat classEnum){

	return pythonClassIntegration.IsSelectingFeatsOnLevelup(handle, classEnum);
}

void D20ClassSystem::LevelupGetBonusFeats(objHndl handle, Stat classEnum){
	pythonClassIntegration.LevelupGetBonusFeats(handle, classEnum);
}

bool D20ClassSystem::IsSelectingFeaturesOnLevelup(objHndl handle, Stat classEnum)
{
	return pythonClassIntegration.IsSelectingFeaturesOnLevelup(handle, classEnum);
}

bool D20ClassSystem::IsSelectingSpellsOnLevelup(objHndl handle, Stat classEnum){

	return pythonClassIntegration.IsSelectingSpellsOnLevelup(handle, classEnum);
}

void D20ClassSystem::LevelupInitSpellSelection(objHndl handle, Stat classEnum, int classLvlNew, int classLvlIncrease){
	// default is -1 (is so when called from the UiCharEditor SpellsActivate)
	auto existingClassLvl = objects.StatLevelGet(handle, classEnum);
	if (classLvlNew == -1 && existingClassLvl )
		dispatch.DispatchLevelupSystemEvent(handle, classEnum, DK_LVL_Spells_Activate); // when advancing the spell level
	else {
		pythonClassIntegration.LevelupInitSpellSelection(handle, classEnum, classLvlNew);
	}
		
}

int D20ClassHooks::HookedLvl1SkillPts(int intStatLvl) {
	auto &selPkt = chargen.GetCharEditorSelPacket();
	auto classCode = selPkt.classCode;
	auto race = selPkt.raceId;
	auto result = NumSkillPointsPerLevel(intStatLvl, race, classCode);

	// Adjust, because this hook doesn't replace the portion that adds 4 for
	// race_human.
	if (race == race_human) result--;

	return result;
}

int D20ClassHooks::NumSkillPointsPerLevel(int intScore, Race race, Stat cls)
{
	auto result = objects.GetModFromStatLevel(intScore);
	auto critter = chargen.GetEditedChar();
	result += d20Sys.D20QueryPython(critter, "Bonus Skillpoints");
	result += d20ClassSys.GetSkillPts(cls);
	if (result < 1)
		result = 1;
	return result;
}

