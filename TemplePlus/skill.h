#pragma once

#include <temple/dll.h>

#include "common.h"

struct BonusSystem;
struct SkillProps;
enum SkillEnum : uint32_t;

struct SkillSystem : temple::AddressTable
{
	SkillProps * skillPropsTable;

	BonusSystem * bonus;

	SkillSystem();
};

extern SkillSystem skillSys;


struct SkillProps
{
	uint32_t classFlags; // bitfield for class skill indication. Is in alphabetic order:  0x1 = barbarian, 0x2 = bard, 0x4 = cleric etc.  0x80000000 indicates disabled skill. See skill.mes for skill names
	uint32_t unskilledUseAllow; // if 0 then it requires at least 1 whole rank in the skill; if 1 it's "free for all" to at least try
	Stat stat; // associated stat (e.g. stat_intelligence for Appraise)
};

enum SkillEnum : uint32_t
{
	skill_appraise = 0,
	skill_bluff = 1,
	skill_concentration = 2,
	skill_diplomacy = 3,
	skill_disable_device = 4,
	skill_gather_information = 5,
	skill_heal = 6,
	skill_hide = 7,
	skill_intimidate = 8,
	skill_listen = 9,
	skill_move_silently = 10,
	skill_open_lock = 11,
	skill_pick_pocket = 12,
	skill_search = 13,
	skill_sense_motive = 14,
	skill_spellcraft = 15,
	skill_spot = 16,
	skill_use_magic_device = 17,
	skill_tumble = 18,
	skill_wilderness_lore = 19,
	skill_perform = 20,
	skill_alchemy = 21,
	skill_balance = 22,
	skill_climb = 23,
	skill_craft = 24,
	skill_decipher_script = 25,
	skill_disguise = 26,
	skill_escape_artist = 27,
	skill_forgery = 28,
	skill_handle_animal = 29,
	skill_innuendo = 30,
	skill_intuit_direction = 31,
	skill_jump = 32,
	skill_knowledge_arcana = 33,
	skill_knowledge_religion = 34,
	skill_knowledge_nature = 35,
	skill_knowledge_all = 36,
	skill_profession = 37,
	skill_read_lips = 38,
	skill_ride = 39,
	skill_swim = 40,
	skill_use_rope = 41
};