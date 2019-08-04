from toee import *
import char_class_utils

###################################################

def GetConditionName():
	return "Hierophant"

def GetSpellCasterConditionName():
	return "Hierophant Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_HIEROPHANTS"

classEnum = stat_level_hierophant

###################################################


class_feats = {
}

class_skills = (skill_alchemy, skill_concentration, skill_craft, skill_diplomacy, skill_heal, skill_knowledge_arcana, skill_knowledge_religion, skill_profession, skill_spellcraft)


def IsEnabled():
	return 0

def GetHitDieType():
	return 8

def GetSkillPtsPerLevel():
	return 2
	
def GetBabProgression():
	return base_attack_bonus_non_martial

def IsFortSaveFavored():
	return 1

def IsRefSaveFavored():
	return 0

def IsWillSaveFavored():
	return 1
def GetSpellListType():
	return spell_list_type_none # hierophants don't advance spell slots, only caster level, which will be done manually in the class condition

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats
	
def IsAlignmentCompatible( alignment):
	return 1


def HasMetamagicFeat(obj):
	metamagic_feats = (feat_empower_spell, feat_enlarge_spell, feat_extend_spell, feat_heighten_spell, feat_maximize_spell, feat_silent_spell, feat_quicken_spell , feat_still_spell, feat_widen_spell, feat_persistent_spell)
	for p in metamagic_feats:
		if obj.has_feat(p):
			return 1
	return 0 
	
def ObjMeetsPrereqs( obj ):
	return 0 # WIP
	if (obj.divine_spell_level_can_cast() < 7):
		return 0
	if (not HasMetamagicFeat(obj)):
		return 0
	if (not obj.has_feat(feat_martial_weapon_proficiency_all) ):
		return 0
	return 1