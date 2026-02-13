#include "stdafx.h"
#include "d20stats.h"
#include <gamesystems/gamesystems.h>
#include <gamesystems/objects/objsystem.h>
#include <feat.h>
#include <util/fixes.h>
#include <d20.h>
#include <critter.h>
#include <config/config.h>

#define VANILLA_STAT_COUNT 288

D20StatsSystem d20Stats;

class D20StatsHooks : public TempleFix{

	void apply() override {
		
		replaceFunction<BOOL(Alignment, Alignment)>(0x1004A8F0, [](Alignment a, Alignment b)->BOOL{
			if (config.laxRules && config.disableAlignmentRestrictions)
				return TRUE;

			return (BOOL)d20Stats.AlignmentsUnopposed(a, b);
		});

		replaceFunction<const char*(Stat)>(0x10073A20, [](Stat stat)->const char* {
			return d20Stats.GetStatRulesString(stat);
		});
		replaceFunction<const char*(Stat)>(0x10074950, [](Stat stat)->const char*{
			return d20Stats.GetStatName(stat);
		});
		replaceFunction<const char*(Race)>(0x10073A50, [](Race race)->const char*{
			return d20Stats.GetRaceName(race);
		});
		replaceFunction<const char*(Race)>(0x10073AB0, [](Race race)->const char* {
			return d20Stats.GetRaceShortDesc(race);
		});
		replaceFunction<const char*(Stat)>(0x10073AE0, [](Stat stat)->const char* {
			return d20Stats.GetClassShortDesc(stat);
		});
		replaceFunction<const char*(Stat)>(0x10073B10, [](Stat stat)->const char* {
			return d20Stats.GetCannotPickClassHelp(stat);
		});

		replaceFunction<int(objHndl, Stat)>(0x10073E90, [](objHndl handle, Stat stat)->int	{
			return d20Stats.GetLevelStat(handle, stat);
		});

		replaceFunction<int(objHndl, Stat)>(0x10074B30, [](objHndl handle, Stat stat){
			if (!handle)
				return 0;
			return d20Stats.GetPhysicalStatBase(handle, stat);
		});

		// ObjStatBaseGet
		static int (__cdecl*orgGetLevelBase)(objHndl, Stat) =
			replaceFunction<int(objHndl, Stat)>(0x10074CF0, [](objHndl handle, Stat stat)->int {
				switch (d20Stats.GetType(stat))
				{
				case StatType::Abilities:
					return objSystem->GetObject(handle)->GetInt32(obj_f_critter_abilities_idx, stat);
				case StatType::Level:
					return d20Stats.GetValue(handle, stat);
				case StatType::Feat:
					return feats.HasFeatCountByClass(handle, static_cast<feat_enums>(stat - 1000));
				case StatType::Physical:
					return d20Stats.GetPhysicalStatBase(handle, stat);
				case StatType::Psi:
					return d20Stats.GetPsiStatBase(handle, stat);
				default:
					return orgGetLevelBase(handle, stat);
				}
			});

		// StatLevelGet
		static int(__cdecl*orgGetLevel)(objHndl, Stat)  = replaceFunction<int(objHndl, Stat)>(0x10074800, [](objHndl handle, Stat stat)->int {
			switch (d20Stats.GetType(stat))
			{
			case StatType::AbilityMods:
				return d20Stats.GetValue(handle, stat);
			case StatType::Level:
				return d20Stats.GetValue(handle, stat);
			case StatType::SpellCasting:
				return d20Stats.GetValue(handle, stat);
			case StatType::Physical:
				return d20Stats.GetPhysicalStatLevel(handle, stat);
			case StatType::Psi:
				return d20Stats.GetPsiStat(handle, stat);
			default:
				return orgGetLevel(handle, stat);
			}
		});

		replaceFunction<const char*(Stat)>(0x10074980, [](Stat stat)->const char*{
			return d20Stats.GetStatShortName(stat);
		});

		replaceFunction<int(objHndl, Stat)>(0x100749B0, [](objHndl handle, Stat classLeveled)->int {
			return d20Stats.GetBaseAttackBonus(handle, classLeveled);
		});

	}
} d20StatsHooks;


const char* D20StatsSystem::GetStatName(Stat stat) const{
	if (GetType(stat) == StatType::Feat){
		return feats.GetFeatName(static_cast<feat_enums>(static_cast<int>(stat) - 1000));
	} 
	return statMesStrings[stat];
}

const char * D20StatsSystem::GetStatShortName(Stat stat) const
{
	if (GetType(stat) == StatType::Feat) {
		return feats.GetFeatName(static_cast<feat_enums>(static_cast<int>(stat) - 1000));
	}
	return statShortNameStrings[stat];
}

const char* D20StatsSystem::GetStatEnumString(Stat stat) const{
	return statEnumStrings[stat];
}

const char* D20StatsSystem::GetStatRulesString(Stat stat) const{
	return statRulesStrings[stat];
}

const char* D20StatsSystem::GetClassShortDesc(Stat stat) const{
	MesLine line(13000 + stat - stat_level_barbarian);
	if (stat <= stat_level_wizard)
		mesFuncs.GetLine_Safe(statMes, &line);
	else
		mesFuncs.GetLine_Safe(statMesExt, &line);
	return line.value;
}

const char * D20StatsSystem::GetAlignmentName(Alignment alignment) {
	return temple::GetRef<const char*[]>(0x10AAE89C)[alignment];
}

const char * D20StatsSystem::GetRaceName(Race race) {

	if (race < VANILLA_NUM_RACES){
		MesLine line(2000 + race);
		mesFuncs.GetLine_Safe(statMes, &line);
		return line.value;
	}
	
	MesLine line(2000 + race);
	mesFuncs.GetLine_Safe(statMesExt, &line);
	return line.value;

}

const char * D20StatsSystem::GetRaceShortDesc(Race race)
{
	MesLine line(12000 + race);
	if (race <= VANILLA_NUM_RACES)
		mesFuncs.GetLine_Safe(statMes, &line);
	else
		mesFuncs.GetLine_Safe(statMesExt, &line);
	return line.value;
}

const char* D20StatsSystem::GetMonsterSubcategoryName(int monsterSubcat){
	MesLine line(8000 + monsterSubcat);
	mesFuncs.GetLine_Safe(statMes, &line);
	return line.value;
}

const char* D20StatsSystem::GetMonsterCategoryName(int monsterCat){
	MesLine line(7000 + monsterCat);
	mesFuncs.GetLine_Safe(statMes, &line);
	return line.value;
}

const char * D20StatsSystem::GetGenderName(int genderId) {
	return temple::GetRef<const char*[]>(0x10AAE410)[genderId];
}

const char* D20StatsSystem::GetCannotPickClassHelp(Stat stat) const{
	
	if (stat <= stat_level_wizard){
		MesLine line(13100 + stat - stat_level_barbarian);
		mesFuncs.GetLine_Safe(statMes, &line);
		return line.value;
	}

	MesLine line(20000 + stat );
	mesFuncs.GetLine_Safe(statMesExt, &line);
	return line.value;
}

int D20StatsSystem::GetValue(const objHndl & handle, Stat stat, int statArg) const
{
	switch (GetType(stat)){
	case StatType::Abilities:
		return objects.abilityScoreLevelGet(handle, stat);
	case StatType::Level:
		return GetLevelStat(handle, stat);
	case StatType::Money:
		return objects.StatLevelGetBase(handle, stat);
	case StatType::SpellCasting:
		return GetSpellCastingStat(handle, stat, statArg);
	case StatType::Psi:
		return GetPsiStat(handle, stat, statArg);
	case StatType::Physical:
		return GetPhysicalStatLevel(handle, stat);
	case StatType::HitPoints:
		// todo!
	case StatType::AbilityMods:
		// Note: gets the actual ability modifier, _not_ the one capped by
		// armor for dex, for example. Max dex bonus only applies to AC, so it
		// makes little sense to return the dex AC modifier here.
		return objects.GetModFromStatLevel(GetValue(handle, static_cast<Stat>(stat-stat_str_mod)));
	case StatType::Speed:
		//todo
	case StatType::Race:
		//todo
	case StatType::Load:
		// todo
	case StatType::SavingThrows:
		// todo

	default:
		return objects.StatLevelGetBase(handle, stat);
			//temple::GetRef<int(__cdecl)(objHndl, Stat)>(0x10074800)(handle, stat);
	}
	return 0;
}

int D20StatsSystem::GetBaseValue(const objHndl & handle, Stat stat, int statArg) const
{
	switch (GetType(stat)) {
	case StatType::Abilities:
		return objects.StatLevelGetBaseWithModifiers(handle, stat, nullptr);
	case StatType::Physical:
		return GetPhysicalStatBase(handle, stat);
	default:
		return objects.StatLevelGetBase(handle, stat);
	}
}

int D20StatsSystem::GetLevelStat(const objHndl &handle, Stat stat) const
{
	auto obj = gameSystems->GetObj().GetObject(handle);
	auto lvlArr = obj->GetInt32Array(obj_f_critter_level_idx);
	auto numItems = lvlArr.GetSize();
	auto result = 0;
	// get the overall level (not accounting for negative levels yet!)
	if (stat == stat_level){
		for (auto i = 0u; i < numItems; i++){
			auto lvl = lvlArr[i];
			if (GetType((Stat)lvl) == StatType::Level)
				result++;
		}
	} 
	else
	{
		for (auto i = 0u; i < numItems; i++) {
			if (lvlArr[i] == stat)
				result++;
		}
	}
	return result;
}

int D20StatsSystem::GetSpellCastingStat(const objHndl & handle, Stat stat, int statArg) const
{
	int ret = 0;

	if (stat == stat_caster_level && statArg != -1) {
		Stat statCheck = static_cast<Stat>(statArg);
		ret = critterSys.GetCasterLevelForClass(handle, statCheck);
	} else if (stat >= stat_caster_level_barbarian && stat <= stat_caster_level_wizard) {
		// Convert stat from stat_caster_level_X to stat_level_X since the function  GetCasterLevelForClass talkes the later
	    Stat statCheck = static_cast<Stat>(static_cast<int>(stat) - static_cast<int>(stat_caster_level_barbarian) + static_cast<int>(stat_level_barbarian));
		ret = critterSys.GetCasterLevelForClass(handle, statCheck);
	} else if (stat == stat_spell_list_level && statArg != -1){
		ret = objects.StatLevelGet(handle, (Stat)statArg) + critterSys.GetSpellListLevelExtension(handle, (Stat)statArg);
	}
	return ret;
}

int D20StatsSystem::GetBaseAttackBonus(const objHndl & handle, Stat classLeveled) const
{
	return critterSys.GetBaseAttackBonus(handle, classLeveled);
}

int D20StatsSystem::GetPsiStat(const objHndl & handle, Stat stat, int statArg) const
{
	if (stat == stat_psi_points_max){
		return d20Sys.D20QueryPython(handle, "Max Psi");
	}

	if (stat == stat_psi_points_cur) {
		return d20Sys.D20QueryPython(handle, "Current Psi");
	}
	return 0;
}

int D20StatsSystem::GetPsiStatBase(const objHndl & handle, Stat stat, int statArg) const
{
	if (stat == stat_psi_points_max) {
		return d20Sys.D20QueryPython(handle, "Base Max Psi");
	}
	if (stat == stat_psi_points_cur) {
		return d20Sys.D20QueryPython(handle, "Current Psi");
	}
	return 0;
}

int D20StatsSystem::GetPhysicalStatBase(const objHndl & handle, Stat stat) const
{
	if (!handle) {
		return 0;
	}

	switch(stat){
	case stat_ac:
		return critterSys.GetArmorClass(handle);
	case stat_damage_bonus:
		return GetBaseValue(handle, stat_str_mod);
	case stat_domain_1:
		return static_cast<int>(objects.getInt32(handle, obj_f_critter_domain_1));
	case stat_domain_2:
		return static_cast<int>(objects.getInt32(handle, obj_f_critter_domain_2));
	case stat_weight:
		return static_cast<int>(objects.getInt32(handle, obj_f_critter_weight));
	case stat_height:
		return static_cast<int>(objects.getInt32(handle, obj_f_critter_height));
	case stat_deity:
		return static_cast<int>(objects.getInt32(handle, obj_f_critter_deity));
	case stat_race:
		return static_cast<int>(objects.getInt32(handle, obj_f_critter_race) & 0x1F);
	case stat_subrace:
		return static_cast<int>(objects.getInt32(handle, obj_f_critter_race) >> 5); // vanilla didn't bitshift
	case stat_gender:
		return static_cast<int>(objects.getInt32(handle, obj_f_critter_gender));
	case stat_size:
		return objects.GetSize(handle, true);
	case stat_alignment:
		return static_cast<int>(objects.getInt32(handle, obj_f_critter_alignment));
	case stat_experience:
		return static_cast<int>(objects.getInt32(handle, obj_f_critter_experience));
	case stat_attack_bonus:
		return critterSys.GetBaseAttackBonus(handle);
	case stat_melee_attack_bonus:
		return critterSys.GetBaseAttackBonus(handle) + objects.GetModFromStatLevel( objects.StatLevelGet(handle, stat_strength) );
	case stat_ranged_attack_bonus:
		return critterSys.GetBaseAttackBonus(handle) + objects.GetModFromStatLevel( objects.StatLevelGet(handle, stat_dexterity));
	default:
		return 0;
		break;
	}
	
	return 0;
}

int D20StatsSystem::GetPhysicalStatLevel(const objHndl & handle, Stat stat) const
{
	if (!handle) {
		return 0;
	}

	auto curSize = objects.GetSize(handle, false);
	auto baseSize = objects.GetSize(handle, true);
	auto sizeDiff = curSize - baseSize;

	int base = 0;

	// TODO: polymorph
	switch(stat) {
	case stat_size:
		return curSize;
	case stat_height:
		base = static_cast<int>(objects.getInt32(handle, obj_f_critter_height));
		// height roughly doubles for every size category
		if (sizeDiff > 0) return base << sizeDiff;
		if (sizeDiff < 0) return base >> (-sizeDiff);
		return base;
	case stat_weight:
		base = static_cast<int>(objects.getInt32(handle, obj_f_critter_weight));
		if (sizeDiff > 0) return base << (3*sizeDiff);
		if (sizeDiff < 0) return base >> (-3*sizeDiff);
		return base;
	case stat_attack_bonus:
	case stat_melee_attack_bonus:
		return critterSys.GetAttackBonus(handle);
	case stat_ranged_attack_bonus:
		return critterSys.GetAttackBonus(handle, D20CAF_RANGED);
	case stat_ac:
		return critterSys.GetArmorClass(handle);
	case stat_damage_bonus:
		return GetValue(handle, stat_str_mod);
	default:
		return GetPhysicalStatBase(handle, stat);
	}
}

bool D20StatsSystem::AlignmentsUnopposed(Alignment a, Alignment b, bool strictCheck){
	if (config.laxRules && config.disableAlignmentRestrictions && !strictCheck)
		return true;

	switch (a^b)
	{
		// XOR operation 
		// example:
		// LAWFUL_GOOD ^ LAWFUL_EVIL      -> GOOD | EVIL -> no 
		// LAWFUL_GOOD ^ LAWFUL_NEUTRAL   -> GOOD        -> ok
	case ALIGNMENT_LAWFUL:
	case ALIGNMENT_CHAOTIC:
	case ALIGNMENT_GOOD:
	case ALIGNMENT_EVIL:
	
		return true;
	default:
		return a == b;
	}
}

void D20StatsSystem::Init(const GameSystemConf& conf){
	isEditor = conf.editor != 0;
	mesFuncs.Open("mes\\stat.mes", &statMes);
	mesFuncs.Open("mes\\stat_ext.mes", &statMesExt);

	mesFuncs.Open("rules\\stat.mes", &statRules);
	mesFuncs.Open("rules\\stat_ext.mes", &statRulesExt);
	mesFuncs.Open("rules\\stat_enum.mes", &statEnum);
	

	for (auto i = 0u; i < _stat_count; i++){
		MesLine line(i);
		if (mesFuncs.GetLine(statMes, &line)){
			statMesStrings[i] = line.value;
		}
		if (mesFuncs.GetLine(statMesExt, &line)) {
			statMesStrings[i] = line.value;
		}

		if (mesFuncs.GetLine(statEnum, &line)){
			statEnumStrings[i] = line.value;
		}

		if (mesFuncs.GetLine(statRules, &line)) {
			statRulesStrings[i] = line.value;
			if (i<VANILLA_STAT_COUNT)
				temple::GetRef<const char*[288]>(0x118CE080)[i] = line.value;
		}
		if (mesFuncs.GetLine(statRulesExt, &line)) {
			statRulesStrings[i] = line.value;
			if (i<VANILLA_STAT_COUNT)
				temple::GetRef<const char*[288]>(0x118CE080)[i] = line.value;
		}
		MesLine shortNameLine(i + 1000);
		if (mesFuncs.GetLine(statMes, &shortNameLine)){
			statShortNameStrings[i] = shortNameLine.value;
		}
		// overrun from the extension file
		if (mesFuncs.GetLine(statMesExt, &shortNameLine)) {
			statShortNameStrings[i] = shortNameLine.value;
		}
		if (!statShortNameStrings[i]){
			mesFuncs.GetLine_Safe(statMesExt, &shortNameLine);
			statShortNameStrings[i] = shortNameLine.value;
		}
	}

	for (auto it: d20ClassSys.vanillaClassEnums){
		MesLine line(13100 + it - stat_level_barbarian);
		mesFuncs.GetLine_Safe(statMes, &line);
		cannotPickClassStr[it] = line.value;
	}
}

StatType D20StatsSystem::GetType(Stat stat) {

	if (stat >= 1000 && stat <= 1999) {
		return StatType::Feat;
	} else if (stat >= 2000 && stat <= 2999) {
		return StatType::Race;
	}

	if (stat >= stat_level && stat <=  127) {
		return StatType::Level;
	}

	switch (stat) {
	case stat_strength:
	case stat_dexterity:
	case stat_constitution:
	case stat_intelligence:
	case stat_wisdom:
	case stat_charisma:
		return StatType::Abilities;

	case stat_str_mod:
	case stat_dex_mod:
	case stat_con_mod:
	case stat_int_mod:
	case stat_wis_mod:
	case stat_cha_mod:
		return StatType::AbilityMods; // largely does (StatLevelGet-10)/ 2 with special casing for dex regarding load

	case stat_level:
	case stat_level_barbarian:
	case stat_level_bard:
	case stat_level_cleric:
	case stat_level_druid:
	case stat_level_fighter:
	case stat_level_monk:
	case stat_level_paladin:
	case stat_level_ranger:
	case stat_level_rogue:
	case stat_level_sorcerer:
	case stat_level_wizard:
	case stat_level_arcane_archer :
	case stat_level_arcane_trickster:
	case stat_level_archmage:
	case stat_level_assassin:
	case stat_level_blackguard:
	case stat_level_dragon_disciple:
	case stat_level_duelist:
	case stat_level_dwarven_defender:
	case stat_level_eldritch_knight:
	case stat_level_hierophant:
	case stat_level_horizon_walker:
	case stat_level_loremaster:
	case stat_level_mystic_theurge:
	case stat_level_shadowdancer:
	case stat_level_thaumaturgist:

	case stat_level_warlock:
	case stat_level_favored_soul:
	case stat_level_red_avenger:
	case stat_level_iaijutsu_master:
	case stat_level_sacred_fist:
	case stat_level_stormlord:
	case stat_level_elemental_savant:
	case stat_level_blood_magus:
	case stat_level_beastmaster:
	case stat_level_cryokineticist:
	case stat_level_frost_mage:
	case stat_level_artificer:
	case stat_level_abjurant_champion:
	case stat_level_scout:
	case stat_level_warmage:
	case stat_level_beguilers:
	case stat_level_swashbuckler:

	case stat_level_psion:
	case stat_level_psychic_warrior:
	case stat_level_soulknife:
	case stat_level_wilder:
	case stat_level_cerebmancer:
	case stat_level_elocator:
	case stat_level_metamind:
	case stat_level_psion_uncarnate:
	case stat_level_psionic_fist:
	case stat_level_pyrokineticist:
	case stat_level_slayer:
	case stat_level_thrallherd:
	case stat_level_war_mind:

	case stat_level_crusader:
	case stat_level_swordsage:
	case stat_level_warblade:
	case stat_level_bloodclaw_master:
	case stat_level_bloodstorm_blade:
	case stat_level_deepstone_sentinel:
	case stat_level_eternal_blade:
	case stat_level_jade_phoenix_mage:
	case stat_level_master_of_nine:
	case stat_level_ruby_knight_vindicator:
	case stat_level_shadow_sun_ninja:
	case stat_level_fochlucan_lyrist:
	case stat_level_marshal:
	case stat_level_ultimate_magus:
	case stat_level_unseen_seer:

		return StatType::Level;

	case stat_money:
	case stat_money_pp:
	case stat_money_gp:
	case stat_money_ep:
	case stat_money_sp:
	case stat_money_cp:
		return StatType::Money;

	case stat_save_reflexes:
	case stat_save_fortitude:
	case stat_save_willpower:
		return StatType::SavingThrows; // does a dispatch for saving throw

	case stat_subdual_damage:
	case stat_hp_max:
	case stat_hp_current:
		return StatType::HitPoints;

	case stat_race:
	case stat_gender:
	case stat_height:
	case stat_weight:
	case stat_size:
	case stat_experience:
	case stat_alignment:
	case stat_deity:
	case stat_domain_1:
	case stat_domain_2:
	case stat_ac:
	case stat_attack_bonus:
	case stat_damage_bonus:
	case stat_subrace:
	case stat_melee_attack_bonus:
	case stat_ranged_attack_bonus:
		return StatType::Physical;

	case stat_movement_speed:
	case stat_run_speed:
		return StatType::Speed; // regards load
		
	case stat_load:
		return StatType::Load;

	case stat_caster_level: 
	case stat_caster_level_barbarian: 
	case stat_caster_level_bard: 
	case stat_caster_level_cleric: 
	case stat_caster_level_druid: 
	case stat_caster_level_fighter: 
	case stat_caster_level_monk: 
	case stat_caster_level_paladin: 
	case stat_caster_level_ranger: 
	case stat_caster_level_rogue: 
	case stat_caster_level_sorcerer: 
	case stat_caster_level_wizard:
	case stat_spell_list_level:
		return StatType::SpellCasting;

	case stat_psi_points_max:
	case stat_psi_points_cur:
		return StatType::Psi;

	default:
		return StatType::Other;
	}

	// the following are unimplemented
	
	//case stat_age: 
	//case stat_category: 
	//case stat_alignment_choice: 
	//
	//
	//case stat_initiative_bonus: 
	//
	//case stat_carried_weight: 

	//case stat_favored_enemies: 
	//case stat_known_spells: 
	//case stat_memorized_spells: 
	//case stat_spells_per_day: 
	//case stat_school_specialization: 
	//case stat_school_prohibited: 

}
