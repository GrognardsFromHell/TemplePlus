from toee import *
import char_class_utils

###################################################

def GetConditionName():
	return "Loremaster"

def GetSpellCasterConditionName():
	return "Loremaster Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_LOREMASTERS"

classEnum = stat_level_loremaster

###################################################


class_feats = {
}

class_skills = (skill_alchemy, skill_appraise, skill_concentration, skill_alchemy, skill_decipher_script, skill_gather_information, skill_handle_animal, skill_heal, skill_knowledge_all, skill_perform, skill_profession, skill_spellcraft, skill_use_magic_device)

def IsEnabled():
	return 0

def GetHitDieType():
	return 4

def GetSkillPtsPerLevel():
	return 4
	
def GetBabProgression():
	return base_attack_bonus_non_martial

def IsFortSaveFavored():
	return 0

def IsRefSaveFavored():
	return 0

def IsWillSaveFavored():
	return 1
def GetSpellListType():
	return spell_list_type_any

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats
	
def IsAlignmentCompatible( alignment):
	return 1


def LoremasterFeatPrereq(obj):
	numFeats = 0
	loremasterFeats = (feat_empower_spell, feat_enlarge_spell, feat_extend_spell, feat_heighten_spell, feat_maximize_spell, feat_silent_spell, feat_quicken_spell , feat_still_spell, feat_widen_spell, feat_persistent_spell, feat_scribe_scroll, feat_brew_potion, feat_craft_magic_arms_and_armor, feat_craft_rod, feat_craft_staff, feat_craft_wand, feat_craft_wondrous_item)
	for p in loremasterFeats:
		if obj.has_feat(p):
			numFeats = numMmFeats + 1
	if (numFeats >= 3):
		return 1
	return 0 

def ObjMeetsPrereqs( obj ):
	return 0 # WIP
	if (not LoremasterFeatPrereq(obj)):
		return 0
	if (obj.stat_level_get(stat_level) < 7): # in lieu of Knowledge ranks
		return 0
	# todo check seven divination spells... bah..
	return 1