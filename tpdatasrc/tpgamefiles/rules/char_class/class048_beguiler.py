from toee import *
import char_class_utils
import char_editor
import tpdp
import functools

# Beguiler:  Player's Handbook 2, p. 6

###################################################

def GetConditionName(): # used by API
	return "Beguiler"

def GetCategory():
	return "Player's Handbook 2"

def GetClassDefinitionFlags():
	return CDF_BaseClass

def GetClassHelpTopic():
	return "TAG_BEGUILERS"
	
classEnum = stat_level_beguiler

###################################################

class_feats = {
1: (feat_armor_proficiency_light, feat_simple_weapon_proficiency, feat_martial_weapon_proficiency_shortbow, feat_exotic_weapon_proficiency_hand_crossbow, feat_martial_weapon_proficiency_short_sword, feat_martial_weapon_proficiency_rapier, feat_traps, "Beguiler Armored Mage"),
2: ("Cloaked Casting", "Surprise Casting"),
5: (feat_silent_spell,),
10: (feat_still_spell,),
}

class_skills = (skill_alchemy, skill_appraise, skill_balance, skill_bluff, skill_climb, skill_concentration, skill_decipher_script, skill_diplomacy, skill_disable_device, skill_disguise, skill_escape_artist, skill_forgery, skill_gather_information, skill_hide, skill_jump, skill_listen, skill_move_silently, skill_open_lock, skill_search, skill_sense_motive, skill_pick_pocket, skill_spellcraft, skill_spot, skill_swim, skill_tumble, skill_use_magic_device, skill_use_rope)

spells_per_day = {
1:  (5, 3),
2:  (6, 4),
3:  (6, 5),
4:  (6, 6, 3),
5:  (6, 6, 4),
6:  (6, 6, 5, 3),
7:  (6, 6, 6, 4),
8:  (6, 6, 6, 5, 3),
9:  (6, 6, 6, 6, 4),
10: (6, 6, 6, 6, 5, 3),
11: (6, 6, 6, 6, 6, 4),
12: (6, 6, 6, 6, 6, 5, 3),
13: (6, 6, 6, 6, 6, 6, 4),
14: (6, 6, 6, 6, 6, 6, 5, 3),
15: (6, 6, 6, 6, 6, 6, 6, 4),
16: (6, 6, 6, 6, 6, 6, 6, 5, 3),
17: (6, 6, 6, 6, 6, 6, 6, 6, 4),
18: (6, 6, 6, 6, 6, 6, 6, 6, 5, 3),
19: (6, 6, 6, 6, 6, 6, 6, 6, 6, 4),
20: (6, 6, 6, 6, 6, 6, 6, 6, 6, 5)
#lvl 0  1  2  3  4  5  6  7  8  9
}

# There are non core spells that are not listed since they are not in constants.py.  If they are added to the game they should be added here.
# Beguilers are granted all the spells on their list as soon as they get atleast one spell per day of that level.  Additional spells are added
# to this list with advanced learning.
spell_list = {
	0: (spell_dancing_lights, spell_daze, spell_detect_magic, spell_ghost_sound, spell_message, spell_open_close, spell_read_magic),
	1: (spell_charm_person, spell_color_spray, spell_comprehend_languages, spell_detect_secret_doors, spell_disguise_self, spell_expeditious_retreat, spell_hypnotism, spell_mage_armor, spell_obscuring_mist, spell_silent_image, spell_sleep, spell_undetectable_alignment, spell_rouse, spell_whelm),
	2: (spell_blur, spell_daze_monster, spell_detect_thoughts, spell_fog_cloud, spell_glitterdust, spell_hypnotic_pattern, spell_invisibility, spell_knock, spell_minor_image, spell_mirror_image, spell_misdirection, spell_see_invisibility, spell_silence, spell_spider_climb, spell_vertigo, spell_whelming_blast),
	3: (spell_clairaudience_clairvoyance, spell_deep_slumber, spell_dispel_magic, spell_displacement, spell_glibness, spell_haste, spell_hold_person, spell_invisibility_sphere, spell_major_image, spell_nondetection, spell_slow, spell_suggestion, spell_inevitable_defeat, spell_greater_mirror_image),
	4: (spell_charm_monster, spell_confusion, spell_crushing_despair, spell_freedom_of_movement, spell_improved_invisibility, spell_locate_creature, spell_rainbow_pattern, spell_solid_fog, spell_mass_whelm),
	5: (spell_break_enchantment, spell_dominate_person, spell_feeblemind, spell_hold_monster, spell_mind_fog, spell_rarys_telepathic_bond, spell_seeming, spell_sending),
	6: (spell_greater_dispelling, spell_mislead, spell_repulsion, spell_shadow_walk, spell_true_seeing, spell_veil),
	7: (spell_ethereal_jaunt, spell_mass_hold_person, spell_mass_invisibility, spell_phase_door, spell_power_word_blind, spell_project_image, spell_spell_turning),
	8: (spell_demand, spell_discern_location, spell_mind_blank, spell_power_word_stun, spell_screen, spell_moment_of_prescience),
	9: (spell_dominate_monster, spell_etherealness, spell_foresight, spell_mass_hold_monster, spell_power_word_kill, spell_time_stop)
}

def IsEnabled():
	return 1

def GetHitDieType():
	return 6
	
def GetSkillPtsPerLevel():
	return 6
	
def GetBabProgression():
	return base_attack_bonus_type_non_martial
	
def IsFortSaveFavored():
	return 0
	
def IsRefSaveFavored():
	return 0
	
def IsWillSaveFavored():
	return 1

# Spell casting
def GetSpellListType():
	return spell_list_type_special

def GetSpellSourceType():
	return spell_source_type_arcane

def GetSpellReadyingType():
	return spell_readying_innate
	
def HasArmoredArcaneCasterFeature():
	return 1

def HasAdvancedLearning():
	return 1
	
def GetAdvancedLearningSpellLevel(spellEnum):
	spEntry = tpdp.SpellEntry(spellEnum)
	spellLevel = spEntry.level_for_spell_class(tpdp.class_enum_to_casting_class(stat_level_wizard))
	return spellLevel
	
def GetSpellList():
	return spell_list

def GetSpellsPerDay():
	return spells_per_day

def GetSpellDeterminingStat():
	return stat_intelligence

def GetSpellDcStat():
	return stat_intelligence

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats

def IsAlignmentCompatible(alignment):
	return 1

def ObjMeetsPrereqs(obj):
	abScore = obj.stat_base_get(stat_intelligence)
	if abScore > 10:
		return 1
	return 0

def GetDeityClass():
	return stat_level_rogue

def IsAdvancedLearningSpell(obj, spell):
	spEntry = tpdp.SpellEntry(spell.spell_enum)

	#Don't add spells that are already known to the list
	if obj.is_spell_known(spell.spell_enum):
		return False

	#First get rid of everything in the beguiler spell list
	for level, spell_list_level in spell_list.items():
		if spell.spell_enum in spell_list_level:
			return False
	
	#Next, get rid of everything that is not enchantment or illustion
	if spEntry.spell_school_enum == Enchantment or spEntry.spell_school_enum == Illusion:
		return True
	
	return False

def GetAdvancedLearningList(obj, maxSpellLevel):
	#Add wizard spells and remove all that are not enchantment or illusion
	
	spAdvancedLearningList = char_editor.get_learnable_spells(obj, stat_level_wizard, maxSpellLevel)
	spAdvancedLearningList = filter(functools.partial(IsAdvancedLearningSpell, obj), spAdvancedLearningList)
		
	for idx in range(0, len(spAdvancedLearningList)):
		spAdvancedLearningList[idx].set_casting_class(classEnum)
	
	return spAdvancedLearningList

def IsSelectingSpellsOnLevelup(obj):
	if char_editor.get_class_code() !=  classEnum: # This is strictly a beguiler benefit (in case it's being accessed externally from something like Mystic Theurge / Archmage)
		return 0
	
	classLvl = obj.stat_level_get(classEnum)
	classLvlNew = classLvl + 1
	
	#levels 3, 7, 11, 15 and 19 get advanced learning otherwise there is nothing to select
	if classLvlNew in [3, 7, 11, 15, 19]:
		maxSpellLvl = char_editor.get_max_spell_level( obj, classEnum, classLvlNew )
		AdvancedLearningList = GetAdvancedLearningList(obj, maxSpellLvl)
		if AdvancedLearningList:
			return 1
	return 0
	
def InitSpellSelection(obj, classLvlNew = -1, classLvlIncrement = 1):
	if char_editor.get_class_code() !=  classEnum: # This is strictly a beguiler benefit (in case it's being accessed externally from something like Mystic Theurge / Archmage)
		return 0

	classLvl = obj.stat_level_get(classEnum)
	if classLvlNew <= 0:
		classLvlNew = classLvl + 1
	maxSpellLvl = char_editor.get_max_spell_level( obj, classEnum, classLvlNew )
	
	spAvail = GetAdvancedLearningList(obj, maxSpellLvl)
	
	# Add the spell level labels
	for p in range(0,maxSpellLvl+1):
		spAvail.append(char_editor.KnownSpellInfo(spell_label_level_0 + p, 0, classEnum))
	spAvail.sort()

	char_editor.append_available_spells(spAvail)
	
	# Add a single spell slot
	vacant_slot = char_editor.KnownSpellInfo(spell_vacant, 3, classEnum)
	spEnums = [vacant_slot]
	char_editor.append_spell_enums(spEnums)
	return 0
	
def LevelupCheckSpells(obj):
	spell_enums = char_editor.get_spell_enums()
	for spInfo in spell_enums:
		if spInfo.spell_enum == spell_vacant:
			return 0
	return 1

def LevelupSpellsFinalize(obj, classLvlNew = -1):
	#Add the normal spells
	classLvl = obj.stat_level_get(classEnum)
	if classLvlNew <= 0:
		classLvlNew = classLvl + 1

	maxSpellLvl = char_editor.get_max_spell_level( obj, classEnum, classLvlNew )
	class_spells = char_editor.get_learnable_spells(obj, classEnum, maxSpellLvl)
	char_editor.spell_known_add(class_spells) # internally takes care of duplicates and the labels/vacant slots	
	
	#Add Anything from advanced learning
	spEnums = char_editor.get_spell_enums()
	char_editor.spell_known_add(spEnums) # internally takes care of duplicates and the labels/vacant slots	
	
	return 0
