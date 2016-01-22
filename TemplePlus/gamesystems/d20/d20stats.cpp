
#include "stdafx.h"
#include "d20stats.h"

StatType D20StatsSystem::GetType(Stat stat) {

	if (stat >= 1000 && stat <= 1999) {
		return StatType::Feat;
	} else if (stat >= 2000 && stat <= 2999) {
		return StatType::Race;
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
		return StatType::AbilityMods;

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
		return StatType::Level;

	case stat_money:
	case stat_money_pp:
	case stat_money_gp:
	case stat_money_sp:
	case stat_money_cp:
		return StatType::Money;

	case stat_save_reflexes:
	case stat_save_fortitude:
	case stat_save_willpower:
		return StatType::SavingThrows;

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
		return StatType::Combat;

	case stat_movement_speed:
	case stat_run_speed:
		return StatType::Speed;
		
	case stat_load:
		return StatType::Load;

	default:
		return StatType::Other;
	}

}
