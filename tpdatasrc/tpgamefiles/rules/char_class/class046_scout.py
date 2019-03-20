from toee import *
import char_class_utils
import char_editor

# Scout:  Complete Adventurer, p. 10

###################################################

def GetConditionName(): # used by API
	return "Scout"

def GetCategory():
	return "Complete Adventurer"

def GetClassDefinitionFlags():
	return CDF_BaseClass

#Located in tpdata\tpmes\help_extensions.tab
def GetClassHelpTopic():
	return "TAG_SCOUTS"

classEnum = stat_level_scout

###################################################

class_feats = {
1: ("Skirmish", feat_armor_proficiency_light, feat_simple_weapon_proficiency, feat_martial_weapon_proficiency_throwing_axe, feat_martial_weapon_proficiency_handaxe, feat_martial_weapon_proficiency_shortbow, feat_martial_weapon_proficiency_short_sword, feat_traps,),
2: ("Battle Fortitude", feat_uncanny_dodge,),
3: ("Fast Movement Scout",),
5: (feat_evasion,),
14: ("Hide in Plain Sight Scout",),
18: ("Free Movement",),
20: ("Blindsight",),
}

bonus_feats = [feat_acrobatic, feat_agile, feat_alertness, feat_athletic, feat_blind_fight, feat_combat_expertise, feat_dodge, feat_far_shot, feat_great_fortitude,
feat_improved_initiative, feat_iron_will, feat_lightning_reflexes, feat_mobility, feat_point_blank_shot, feat_precise_shot, feat_quick_draw, feat_rapid_reload, 
feat_shot_on_the_run, feat_spring_attack, feat_track,
feat_skill_focus_alchemy,feat_skill_focus_animal_empathy, feat_skill_focus_appraise, feat_skill_focus_balance, feat_skill_focus_bluff,
feat_skill_focus_climb, feat_skill_focus_concentration, feat_skill_focus_craft, feat_skill_focus_decipher_script, feat_skill_focus_diplomacy,
feat_skill_focus_disable_device, feat_skill_focus_disguise, feat_skill_focus_escape_artist, feat_skill_focus_forgery, feat_skill_focus_gather_information, 
feat_skill_focus_handle_animal, feat_skill_focus_heal, feat_skill_focus_hide, feat_skill_focus_innuendo, feat_skill_focus_intimidate, feat_skill_focus_intuit_direction,
feat_skill_focus_jump, feat_skill_focus_knowledge, feat_skill_focus_listen, feat_skill_focus_move_silently, feat_skill_focus_open_lock,
feat_skill_focus_performance, feat_skill_focus_slight_of_hand, feat_skill_focus_profession, feat_skill_focus_read_lips, feat_skill_focus_ride,
feat_skill_focus_scry, feat_skill_focus_search, feat_skill_focus_sense_motive, feat_skill_focus_speak_language, feat_skill_focus_spellcraft, feat_skill_focus_spot,
feat_skill_focus_swim, feat_skill_focus_tumble, feat_skill_focus_use_magic_device, feat_skill_focus_use_rope, feat_skill_focus_survival
]

class_skills = (skill_balance, skill_climb, skill_craft, skill_disable_device, skill_escape_artist, skill_hide, skill_jump, skill_knowledge_nature, skill_listen, skill_move_silently, skill_ride, skill_search, skill_sense_motive, skill_spot, skill_wilderness_lore, skill_swim, skill_tumble, skill_use_rope)

def IsEnabled():
	return 1

def GetHitDieType():
	return 8
	
def GetSkillPtsPerLevel():
	return 8
	
def GetBabProgression():
	return base_attack_bonus_type_semi_martial
	
def IsFortSaveFavored():
	return 0
	
def IsRefSaveFavored():
	return 1
	
def IsWillSaveFavored():
	return 0

def GetSpellListType():
	return spell_list_type_none

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats
	
def IsAlignmentCompatible( alignment):
	return 1

def ObjMeetsPrereqs( obj ):
	return 1
	
def GetDeityClass():
	return stat_level_ranger
	
def IsSelectingFeatsOnLevelup( obj ):
	newLvl = obj.stat_level_get( classEnum ) + 1
	if newLvl % 4 == 0:
		return 1
	return 0
	
def LevelupGetBonusFeats( obj ):
	bonFeatInfo = []
	for ft in bonus_feats:
		bonFeatInfo.append(char_editor.FeatInfo(ft))
	char_editor.set_bonus_feats(bonFeatInfo)
	return
	
def LevelupSpellsFinalize( obj, classLvlNew = -1 ):
	return 0