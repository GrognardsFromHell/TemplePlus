#include "stdafx.h"
#include "util/fixes.h"
#include "common.h"
#include "tig/tig_tabparser.h"
#include "gamesystems/legacymapsystems.h"
#include "gamesystems/gamesystems.h"
#include <infrastructure/elfhash.h>
#include <gamesystems/objects/gameobject.h>
#include <gamesystems/objects/objsystem.h>
#include <condition.h>
#include <tig/tig_tokenizer.h>
#include "weapon.h"
#include "dungeon_master.h"
#include "d20_race.h"

class ProtosHooks : public TempleFix{
public:

	// static int StdParamParserFunc(int colIdx, objHndl handle, char* content, obj_f fieldId, int arrayLen, char** stringArray, int fieldSubIdx); // for future reference
	static int ParseCondition(int colIdx, objHndl handle, char* content, int condIdx, int stage, int unused, int unused2);
	static int ParseMonsterSubcategory(int colIdx, objHndl handle, char* content, obj_f field, int arrayLen, char** strings);
	static int ParseType(int colIdx, objHndl handle, char* content, obj_f field, int arrayLen, char** strings);
	static int ParseRace(int colIdx, objHndl handle, char* content, obj_f field, int arrayLen, char** strings);
	static int ParseSpell(int colIdx, objHndl handle, char* content, obj_f field);

	static int ParseWeaponDamageType(int colIdx, objHndl handle, char* content, obj_f field);
	static int ParseWeaponCritRange(int colIdx, objHndl handle, char* content, obj_f field);
	static int ParseDice(int colIdx, objHndl handle, char* content, obj_f field);
	static int GenericNumber(int colIdx, objHndl handle, char* content, obj_f field);

	static int SetCritterAttacks(objHndl handle);

	static int DamageTypeFromString(char* str);
	static int DamageTypeFromD20String(char* str);

	void apply() override
	{

		// protos.tab line parser; replaced for supporting extensions
		static int(*orgProtoParser)(TigTabParser*, int, const char**) =
			replaceFunction<int(__cdecl)(TigTabParser*, int, const char**)>(0x1003B640, [](TigTabParser* parser, int lineIdx, const char** cols)->int {

			static std::vector<int> parsedProtoIds;

			auto & parsedIds = parsedProtoIds;
			auto protoNum = atol(cols[0]);
			auto foundProto = std::find(parsedIds.begin(), parsedIds.end(), protoNum);
			if (foundProto == parsedIds.end()) {
				parsedIds.push_back(protoNum);
				auto result = orgProtoParser(parser, lineIdx, cols);
				dmSys.InitEntry(protoNum);
				return result;
			}
			else
			{
				logger->info("Skipping duplicate proto {}", protoNum);
			}

			return 0;

		});

		// Fix for protos condition parser
		replaceFunction<int(__cdecl)(int, objHndl, char*, int, int, int, int)>(0x10039E80, ParseCondition);

		// Fix for monster subcateogry extraplanar / extraplaner
		replaceFunction<int(__cdecl)(int, objHndl, char*, obj_f, int, char**)>(0x100398C0, ParseMonsterSubcategory);

		// Hook the generic ParseType function
		replaceFunction<int(__cdecl)(int, objHndl, char*, obj_f, int, char**)>(0x10039680, ParseType);

		replaceFunction<int(__cdecl)(int, objHndl, char*, obj_f, int, char**)>(0x10039B40, ParseRace);

		replaceFunction<int(__cdecl)(int, objHndl, char*, obj_f)>(0x1003A8C0, ParseSpell);

		// Hook into weapon parsing for e.g. shield bash stats
		replaceFunction(0x100397D0, ParseWeaponDamageType);
		replaceFunction(0x10039CE0, ParseWeaponCritRange);
		replaceFunction(0x10039B80, ParseDice);
		replaceFunction(0x10039380, GenericNumber);

		//SetCritterAttacks
		replaceFunction<int(__cdecl)(objHndl)>(0x1003AAC0, SetCritterAttacks);
	}
} protosHooks;

int ProtosHooks::ParseCondition(int colIdx, objHndl handle, char * content, int condIdx, int stage, int unused, int unused2){
	static std::unordered_map<string, int> attributeEnumTable({
		{"attribute_strength", stat_strength},
		{"attribute_dexterity", stat_dexterity},
		{"attribute_constitution", stat_constitution },
		{"attribute_intelligence", stat_intelligence},
		{"attribute_wisdom", stat_wisdom },
		{"attribute_charisma", stat_charisma },
	});
	
	static std::unordered_map<string, int> savingThrowEnumTable({
		{"saving_throw_fortitude", (int)SavingThrowType::Fortitude},
		{"saving_throw_reflex", (int)SavingThrowType::Reflex},
		{"saving_throw_will", (int)SavingThrowType::Will}
	});

	static std::unordered_map<string, int> typeEnumTable({
		{"type_animal", 0},
		{"type_beast", 1 },
		{ "type_construct", 2 },
		{ "type_dragon", 3},
		{ "type_elemental", 4},
		{ "type_fey", 5},
		{ "type_giant", 6},
		{ "type_magical_beast", 7},
		{ "type_monstrous_humanoid", 8},
		{ "type_ooze", 9},
		{ "type_plant", 10},
		{ "type_shapechanger", 11 },
		{ "type_undead", 12 },
		{ "type_vermin", 13 },
		{ "type_outsider_subtype_evil", 14 },
		{ "type_outsider_subtype_good", 15 },
		{ "type_outsider_subetype_lawful", 16 },
		{ "type_outsider_subtype_chaotic", 17 },
		{ "type_humanoid_subtype_goblinoid", 18 },
		{ "type_humanoid_subtype_reptilian", 19 },

		{ "type_humanoid_subtype_dwarf", 20 },
		{ "type_humanoid_subtype_elf", 21},
		{ "type_humanoid_subtype_gnoll", 22 },
		{ "type_humanoid_subtype_gnome", 23 },
		{ "type_humanoid_subtype_halfling", 24 },
		{ "type_humanoid_subtype_orc", 25 },
		{ "type_humanoid_subtype_human", 26 },
		{ "type_humanoid_subtype_", 27 }
	});

	static std::unordered_map<string, int> craftingEnumTable({
		{ "nothing", 0},
		{ "potion", 1},
		{ "scroll", 2},
		{ "wand", 3}
	});

	auto &protoParseCondId = temple::GetRef<int>(0x108EE818);
	auto &protoParseParam1 = temple::GetRef<int>(0x108EDAA8);
	auto &protoParseParam2 = temple::GetRef<int>(0x108EDAA0);
	auto &isParsingCond = temple::GetRef<int>(0x108EE820);
	static int lastStage = 0;

	if (stage == 0){
		lastStage = stage;

		if (content && *content) {
			protoParseCondId = ElfHash::Hash(content);
			protoParseParam1 = 0;
			protoParseParam2 = 0;
			isParsingCond = 1;
		}
		else
		{
			isParsingCond = 0;
		}
		return 1;	
	}
	else if (!isParsingCond)
	{
		return 1;
	}

	Expects(stage == lastStage + 1);
	lastStage = stage;
		

	if (stage == 1)	{

		if (!content || !*content){
			return 1;
		}

		auto findAttrib = attributeEnumTable.find(tolower(content));
		if (findAttrib != attributeEnumTable.end()){
			protoParseParam1 = findAttrib->second;
			return 1;
		}

		auto findSavThrow = savingThrowEnumTable.find(tolower(content));
		if (findSavThrow != savingThrowEnumTable.end()) {
			protoParseParam1 = findSavThrow->second;
			return 1;
		}

		auto findMonType = typeEnumTable.find(tolower(content));
		if (findMonType != typeEnumTable.end()) {
			protoParseParam1 = findMonType->second;
			return 1;
		}

		protoParseParam1 = DamageTypeFromString(content);
		if (protoParseParam1 != -1)
		{
			return 1;
		}

		auto attackPowerFromString = temple::GetRef<int(__cdecl)(char*)>(0x100E15C0);
		protoParseParam1 = attackPowerFromString(content);
		if (protoParseParam1){
			protoParseParam1 |= 0x80000000;
			return 1;
		}

		auto getSkillEnumFromString = temple::GetRef<BOOL(__cdecl)(int*, char*)>(0x1007D2F0);
		if (getSkillEnumFromString(&protoParseParam1, content)){
			return 1;
		}

		std::string txtBuf(content);
		if (txtBuf[0] == '\''){ // unspecific string parameter
			txtBuf.erase(0,1);
			for (auto j=0u; j < txtBuf.size(); j++){
				if (!txtBuf[j] || txtBuf[j] == '\''){
					txtBuf[j] = 0;
					protoParseParam1 = ElfHash::Hash(txtBuf);
					return 1;
				}
			}
			
			protoParseParam1 = ElfHash::Hash(txtBuf);
			return 1;
		}
		else
		{
			protoParseParam1 = atol(content);
			return 1;
		}
		
	}

	if (stage != 2){
		logger->error("Unknown proto condition passed to ParseCondition {}", stage);
		isParsingCond = 0;
		return 1;
	}

	// stage 2
	bool stage2Ok = false;
	if (content && *content){
		
		auto findCrafting = craftingEnumTable.find(tolower(content));
		if (findCrafting != craftingEnumTable.end()) {
			protoParseParam2 = findCrafting->second;
			if (protoParseParam2 != 3)
			{
				auto asd = 1;
			}
			stage2Ok = true;
		}

		if (!stage2Ok){
			protoParseParam2 = DamageTypeFromString(content);
			if (protoParseParam2 != -1)
			{
				stage2Ok = true;
			}
		}
		
		if (!stage2Ok){
			int diceCount, diceType, diceBonus;
			if (Dice::Parse(content, diceCount, diceType, diceBonus)) {
				Dice dice(diceCount, diceType, diceBonus);
				protoParseParam2 = dice.ToPacked();
				stage2Ok = true;
			}
		}
		
		if (!stage2Ok)
		{
			protoParseParam2 = atol(content);
		}

	}

	if (isParsingCond)
	{
		auto obj = objSystem->GetObject(handle);
		auto condField = obj_f_conditions;
		auto condArgField = obj_f_condition_arg0;
		if (!obj->IsCritter())
		{
			condField = obj_f_item_pad_wielder_condition_array;
			condArgField = obj_f_item_pad_wielder_argument_array;
		}
		
		auto condStruct = conds.GetById(protoParseCondId);
		if (!condStruct){
			logger->warn("Unknown proto condition passed to ParseCondition: {}, proto {}, property index {}", protoParseCondId, obj->id.GetPrototypeId(), condIdx);
			isParsingCond = 0;
			return 1;
		}

		auto condArray = obj->GetInt32Array(condField);
		auto condArgArray = obj->GetInt32Array(condArgField);
		
		obj->SetInt32(condField, obj->GetInt32Array(condField).GetSize(), protoParseCondId);

		if (condStruct->numArgs > 0)
			obj->SetInt32(condArgField, obj->GetInt32Array(condArgField).GetSize(), protoParseParam1);
		if (condStruct->numArgs > 1)
			obj->SetInt32(condArgField, obj->GetInt32Array(condArgField).GetSize(), protoParseParam2);
		// fill the rest with 0
		for (auto j = 2u; j < condStruct->numArgs; j++)
			obj->SetInt32(condArgField, obj->GetInt32Array(condArgField).GetSize(), 0);

		isParsingCond = 0;
	}
	return 1;
}

int ProtosHooks::ParseMonsterSubcategory(int colIdx, objHndl handle, char * content, obj_f field, int arrayLen, char ** strings){

	if (!content[0])
		return 1;

	arrayLen = 28; // bug in ToEE - looks like a last minute addition of mc_subtype_water and they forgot to increase the array length

	auto subcatFlags = (uint64_t)0;
	StringTokenizer tok(content);
	while (tok.next()){
		auto& tokItem = tok.token();

		// fix for extraplaner typo
		if (!_strcmpi(tokItem.text, "mc_subtype_extraplaner") || !_strcmpi(tokItem.text, "mc_subtype_extraplanar")){
			subcatFlags |= MonsterSubcategoryFlag::mc_subtype_extraplanar;
			continue;
		}

		// native subtype won't be in the array
		if (!_strcmpi(tokItem.text, "mc_subtype_native")) {
			subcatFlags |= MonsterSubcategoryFlag::mc_subtype_native;
			continue;
		}

		for (auto i=0; i < arrayLen; i++){
			if (!_strcmpi(strings[i], tokItem.text)){
				auto flagTmp = 1 << i;
				subcatFlags |= flagTmp;
			}
		}
	}

	auto subcatTest = (MonsterSubcategoryFlag)subcatFlags;

	auto obj = objSystem->GetObject(handle);
	auto moncat = (uint64_t)obj->GetInt64(obj_f_critter_monster_category);
	moncat |= (subcatFlags << 32);
	obj->SetInt64(obj_f_critter_monster_category, moncat);
	return 1;
}

static bool armorWeaponStats = false;
static int armorWeaponStage = 0;
static int armorWeaponDamageType = (int)DamageType::Bludgeoning;
static int armorWeaponDice = 0;
static int armorWeaponCritRange = 20;
static int armorWeaponCritMult = 2;

// armor weapon condition initialization and reset parsing
void ArmorWeaponFinalize(GameObjectBody* obj) {
	if (armorWeaponStats) {
		// shield bash is the only one right now
		auto condId = ElfHash::Hash("Shield Bash");

		auto condField = obj_f_item_pad_wielder_condition_array;
		auto condArgField = obj_f_item_pad_wielder_argument_array;
		auto condArray = obj->GetInt32Array(condField);
		auto condArgArray = obj->GetInt32Array(condArgField);

		obj->SetInt32(condField, condArray.GetSize(), condId);

		obj->SetInt32(condArgField, condArgArray.GetSize(), armorWeaponDamageType);
		obj->SetInt32(condArgField, condArgArray.GetSize(), armorWeaponDice);
		obj->SetInt32(condArgField, condArgArray.GetSize(), 0);
		obj->SetInt32(condArgField, condArgArray.GetSize(), armorWeaponCritRange);
		obj->SetInt32(condArgField, condArgArray.GetSize(), armorWeaponCritMult);
	}

	armorWeaponStats = false;
	armorWeaponStage = 0;
	armorWeaponDice = 0;
	armorWeaponCritRange = 20;
	armorWeaponCritMult = 2;
	armorWeaponDamageType = (int)DamageType::Bludgeoning;
}

int ProtosHooks::ParseType(int colIdx, objHndl handle, char * content, obj_f field, int arrayLen, char ** strings){

	auto foundType = false;
	auto val = 0;

	if (!content || !*content) return FALSE;

	if (arrayLen <= 0) return 0;

	for (auto i=0; i< arrayLen; i++){
		if ( strings[i] && !_strcmpi(content, strings[i])){
			
			val = i;
			objSystem->GetObject(handle)->SetInt32(field, val);
			foundType = true;
			break;
		}
	}

	auto obj = objSystem->GetObject(handle);

	if (field == obj_f_weapon_type){
		if (!foundType && !_strcmpi(content, "shield")) {
			armorWeaponStats = true;
			return TRUE;
		}

		if (!foundType && !_strcmpi(content, "wt_mindblade")){
			val = wt_mindblade;
			foundType = true;
			obj->SetInt32(field, val);
		}

		auto weapType = (WeaponTypes)val;
		auto damType = weapons.wpnProps[weapType].damType;
		
		if (damType == DamageType::Unspecified){
			weapons.wpnProps[weapType].damType = (DamageType)obj->GetInt32(obj_f_weapon_attacktype);
		}
		//else if (damType != (DamageType)objSystem->GetObject(handle)->GetInt32(obj_f_weapon_attacktype)){ // for debug
		//	auto d = description.getDisplayName(handle);
		//	auto strangeWeaponDamType = (DamageType)objSystem->GetObject(handle)->GetInt32(obj_f_weapon_attacktype);
		//	auto dummy = 1;
		//}
	}
	
	return foundType ? TRUE : FALSE;
}

int ProtosHooks::GenericNumber(int colIdx, objHndl handle, char *content, obj_f field) {
	auto obj = objSystem->GetObject(handle);

	bool armorWeapon = obj_t_armor == obj->type && obj_f_weapon_crit_hit_chart == field;

	if (content && *content) {
		if (armorWeapon) { // override critical hit multiplier for armor
			armorWeaponCritMult = atol(content);
		} else {
			obj->SetInt32(field, atol(content));
		}
	}

	if (armorWeapon && ++armorWeaponStage >= 4)
		ArmorWeaponFinalize(obj);

	return 1;
}

int ProtosHooks::ParseRace(int colIdx, objHndl handle, char * content, obj_f field, int arrayLen, char ** strings)
{
	if (content && *content){
		auto race = d20RaceSys.GetRaceEnum(content);
		objSystem->GetObject(handle)->SetInt32(field, race);
	}
	return 1;
}

int ProtosHooks::ParseSpell(int colIdx, objHndl handle, char* content, obj_f field){
	if (!content || !*content)
		return 1;
	StringTokenizer tok(content);
	if (!tok.next())
		return 1;

	if (tok.token().type != StringTokenType::QuotedString)
		return 1;
	
	
	auto spEnum = spellSys.GetSpellEnum(tok.token().text);
	if (!spEnum){
		logger->error("ParseSpell: attempting to add a non-existant spell {}", content);
	}
	if (!tok.next() || tok.token().type != StringTokenType::Identifier)
		return 1;

	auto spClass = spellSys.GetSpellClass(tok.token().text);
	if (!tok.next() || tok.token().type != StringTokenType::Number)
		return 1;
	auto spLvl = tok.token().numberInt;

	auto obj = objSystem->GetObject(handle);
	if (!obj->IsCritter() || (!spellSys.isDomainSpell(spClass) && spClass != Domain::Domain_Special )){

		
		auto shouldAddKnown = true;
		std::vector<int> spellClasses, spellLevels;
		if (obj->IsCritter() && spellSys.SpellKnownQueryGetData(handle, spEnum, spellClasses, spellLevels)){
			for (auto i=0; i < spellClasses.size(); i++){
				if (spellClasses[i] == spClass && spellLevels[i] == spLvl){
					shouldAddKnown = false;
					break;
				}
			}
		}
		if (shouldAddKnown)
			spellSys.SpellKnownAdd(handle, spEnum, spClass, spLvl, SpellStoreType::spellStoreKnown, 0);
		if (!obj->IsCritter())
			return 1;
	}
	spellSys.SpellMemorizedAdd(handle, spEnum, spClass, spLvl, SpellStoreType::spellStoreMemorized, 0);

	return 1;
}

int ProtosHooks::ParseWeaponDamageType(int colIdx, objHndl handle, char* content, obj_f field) {
	auto obj = objSystem->GetObject(handle);
	bool armorWeapon = obj->type == obj_t_armor && field == obj_f_weapon_attacktype;

	auto dmgType = DamageTypeFromD20String(content);
	if (-1 == dmgType && _strcmpi(content, "D20DT_UNSPECIFIED")) {
		dmgType = atol(content);
	}

	if (content && *content) {
		if (armorWeapon) {
			armorWeaponDamageType = dmgType;
		} else {
			// original behavior
			obj->SetInt32(field, dmgType);
		}
	}

	if (armorWeapon && ++armorWeaponStage >= 4)
		ArmorWeaponFinalize(obj);

	return 1;
}

int ProtosHooks::ParseWeaponCritRange(int colIdx, objHndl handle, char* content, obj_f field) {
	auto obj = objSystem->GetObject(handle);
	bool armorWeapon = obj->type == obj_t_armor && obj_f_weapon_crit_range == field;

	if (content && *content) {
		// Crit ranges in the proto are given as the lowest crit roll,
		// but they are stored as how many sides of the die are critical hits.
		auto range = 21 - atol(content);

		if (armorWeapon) {
			armorWeaponCritRange = range;
		} else {
			obj->SetInt32(field, range);
		}
	}

	if (armorWeapon && ++armorWeaponStage >= 4)
		ArmorWeaponFinalize(obj);

	return 1;
}

int ProtosHooks::ParseDice(int colIdx, objHndl handle, char* content, obj_f field) {
	auto obj = objSystem->GetObject(handle);
	bool armorWeapon = obj->type == obj_t_armor && obj_f_weapon_damage_dice == field;
	int result = 1;

	if (content && *content) {
		int diceCount, diceType, diceBonus;
		if (!Dice::Parse(content, diceCount, diceType, diceBonus)) {
			result = 0;
		} else {
			Dice dice(diceCount, diceType, diceBonus);

			if (armorWeapon) {
				armorWeaponDice = dice.ToPacked();
			} else {
				obj->SetInt32(field, dice.ToPacked());
			}
		}
	}

	if (armorWeapon && ++armorWeaponStage >= 4)
		ArmorWeaponFinalize(obj);

	return result;
}

int ProtosHooks::SetCritterAttacks(objHndl handle)
{
	auto obj = objSystem->GetObject(handle);
	if (obj->IsCritter())
	{
		int strMod = objects.GetModFromStatLevel(obj->GetInt32(obj_f_critter_abilities_idx, 0));
		int sizeMod = critterSys.GetBonusFromSizeCategory(obj->GetInt32(obj_f_size));
		for(int attackIndex = 0; attackIndex < 3; attackIndex++)
		{
			if (obj->GetInt32(obj_f_critter_attacks_idx, attackIndex) > 0)
			{
				int32_t attackBonusOld = obj->GetInt32(obj_f_attack_bonus_idx, attackIndex);
				auto attackBonusNew = attackBonusOld - (strMod + sizeMod);
				obj->SetInt32(obj_f_attack_bonus_idx, attackIndex, attackBonusNew);

				// Decrement dice damage modifier to remove STR bonus, as it will be added later on
				Dice diceDamage = Dice::FromPacked(obj->GetInt32(obj_f_critter_damage_idx, attackIndex));
				int newDiceMod = diceDamage.GetModifier() - strMod / 2;
				if (attackIndex <= 0 || strMod <= 0)
					newDiceMod = diceDamage.GetModifier() - strMod;
				
				Dice diceDamageNew = Dice(diceDamage.GetCount(), diceDamage.GetSides(), newDiceMod);
				obj->SetInt32(obj_f_critter_damage_idx, attackIndex, diceDamageNew.ToPacked());
			}
		}

		// Last Natural Attack (3) is dex based, therefore dex and size is removed from attack bonus
		if (obj->GetInt32(obj_f_critter_attacks_idx, 3) > 0)
		{
			auto dexMod = objects.GetModFromStatLevel(obj->GetInt32(obj_f_critter_abilities_idx, 1));
			auto attackBonusOld = obj->GetInt32(obj_f_attack_bonus_idx, 3);
			
			obj->SetInt32(obj_f_attack_bonus_idx, 3, attackBonusOld - dexMod - sizeMod);
		}
	}
	return 0;
}

int ProtosHooks::DamageTypeFromString(char* str) {
	static auto original = temple::GetRef<int(__cdecl)(char*)>(0x100E0AF0);
	return original(str);
}

int ProtosHooks::DamageTypeFromD20String(char* content) {
	static std::unordered_map<string, DamageType> damageTypeEnumTable({
		{"d20dt_unspecified", DamageType::Unspecified},
		{"d20dt_bludgeoning", DamageType::Bludgeoning},
		{"d20dt_piercing", DamageType::Piercing},
		{"d20dt_slashing", DamageType::Slashing},
		{"d20dt_bludgeoning_and_piercing", DamageType::BludgeoningAndPiercing},
		{"d20dt_piercing_and_slashing", DamageType::PiercingAndSlashing},
		{"d20dt_slashing_and_bludgeoning", DamageType::SlashingAndBludgeoning},
		{"d20dt_slashing_and_bludgeoning_and_piercing", DamageType::SlashingAndBludgeoningAndPiercing},
		{"d20dt_acid", DamageType::Acid},
		{"d20dt_cold", DamageType::Cold},
		{"d20dt_electricity", DamageType::Electricity},
		{"d20dt_fire", DamageType::Fire},
		{"d20dt_sonic", DamageType::Sonic},
		{"d20dt_negative_energy", DamageType::NegativeEnergy},
		{"d20dt_subdual", DamageType::Subdual},
		{"d20dt_poison", DamageType::Poison},
		{"d20dt_positive_energy", DamageType::PositiveEnergy},
		{"d20dt_force", DamageType::Force},
		{"d20dt_blood_loss", DamageType::BloodLoss},
		{"d20dt_magic", DamageType::Magic}
	});

	auto findDT = damageTypeEnumTable.find(tolower(content));
	if (findDT != damageTypeEnumTable.end()){
		return (int)findDT->second;
	}

	// default to unspecified
	return -1;
}
