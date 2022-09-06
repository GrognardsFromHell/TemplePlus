from toee import *
import char_class_utils
import char_editor
import tpdp
###################################################

def GetConditionName():
	return "Ultimate Magus"

def GetSpellCasterConditionName():
	return "Ultimate Magus Spellcasting"

def GetCategory():
	return "Complete Mage Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_None

def GetClassHelpTopic():
	return "TAG_ULTIMATE_MAGUS"

classEnum = stat_level_ultimate_magus

###################################################

class_feats = {
1: ("Arcane Spell Power",),
2: ("Expanded Spell Knowledge",),
3: ("Augmented Casting",),
}

class_skills = (skill_alchemy, skill_concentration, skill_craft, skill_decipher_script, skill_knowledge_arcana, skill_knowledge_religion, skill_knowledge_nature, skill_knowledge_all, skill_profession, skill_spellcraft, skill_use_magic_device)

def IsEnabled():
	return 1

def GetHitDieType():
	return 4

def GetSkillPtsPerLevel():
	return 2
	
def GetBabProgression():
	return base_attack_bonus_type_non_martial

def IsFortSaveFavored():
	return 0

def IsRefSaveFavored():
	return 0

def IsWillSaveFavored():
	return 1

def GetSpellListType():
	return spell_list_type_extender

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats
	
def IsAlignmentCompatible( alignment):
	return 1

def HasMetamagicFeat(obj):
	metamagic_feats = tpdp.get_metamagic_feats()
	for p in metamagic_feats:
		if obj.has_feat(p):
			return 1
	return 0 

def ObjMeetsPrereqs( obj ):
	if not HasMetamagicFeat(obj):
		return 0
	if obj.arcane_vancian_spell_level_can_cast() < 2 or obj.arcane_spontaneous_spell_level_can_cast() < 1:
		return 0
	if obj.skill_ranks_get(skill_spellcraft) < 8:
		return 0
	#Knowledge arcane 4 not enfroced
	return 1


# Levelup

def IsSelectingFeatsOnLevelup( obj ):
	newLvl = obj.stat_level_get( classEnum ) + 1
	if newLvl == 4 or newLvl == 9:
		return 1
	return 0
	
def LevelupGetBonusFeats( obj ):
	bonFeatInfo = []
	feat_list = tpdp.get_metamagic_feats()
	for ft in feat_list:
		bonFeatInfo.append(char_editor.FeatInfo(ft))
	
	char_editor.set_bonus_feats(bonFeatInfo)
	return
	
#Check if which levels get increased.  Returns a flag for vacian and spontaneous.
def CheckUMIncreaseOnLevelup(umLevel, vacianCasterLevel, spontaneousCasterLevel):
	levelUpVacian = true
	levelUpSpontaneous = true
	if umLevel in [1,4,7]:
		if spontaneousCasterLevel < vacianCasterLevel:
			levelUpVacian = false
		else:
			levelUpSpontaneous = false
	return levelUpVacian, levelUpSpontaneous
	
#Check if the class gets spells on this levelup.  Returns a flag for vacian and spontaneous
def ClassNeedsSpellsOnLevelup(obj, class_extended_vancian, class_extended_spontaneous, caster_bonus_vancian, caster_bonus_spontaneous):
	newLvl = obj.stat_level_get( classEnum ) + 1
	vacianLvl = obj.stat_level_get( class_extended_vancian ) + caster_bonus_vancian
	spontaneousLvl = obj.stat_level_get( class_extended_spontaneous ) + caster_bonus_spontaneous
	return CheckUMIncreaseOnLevelup(newLvl, vacianLvl, spontaneousLvl)

def IsSelectingSpellsOnLevelup( obj , class_extended_vancian = 0, class_extended_spontaneous = 0, caster_bonus_vancian = 0, caster_bonus_spontaneous = 0):
	if class_extended_vancian <= 0 or class_extended_spontaneous <= 0:
		class_extended_vancian = char_class_utils.GetHighestVancianArcaneClass(obj)
		class_extended_spontaneous = char_class_utils.GetHighestSpontaneousArcaneClass(obj)
	levelUpVacian, levelUpSpontaneous = ClassNeedsSpellsOnLevelup(obj, class_extended_vancian, class_extended_spontaneous, caster_bonus_vancian, caster_bonus_spontaneous)
	if levelUpVacian:
		if char_editor.is_selecting_spells(obj, class_extended_vancian):
			return 1
	if levelUpSpontaneous:
		if char_editor.is_selecting_spells(obj, class_extended_spontaneous):
			return 1
	return 0

def LevelupCheckSpells( obj , class_extended_vancian = 0, class_extended_spontaneous = 0, caster_bonus_vancian = 0, caster_bonus_spontaneous = 0):
	if class_extended_vancian <= 0 or class_extended_spontaneous <= 0:
		class_extended_vancian = char_class_utils.GetHighestVancianArcaneClass(obj)
		class_extended_spontaneous = char_class_utils.GetHighestSpontaneousArcaneClass(obj)
	levelUpVacian, levelUpSpontaneous = ClassNeedsSpellsOnLevelup(obj, class_extended_vancian, class_extended_spontaneous, caster_bonus_vancian, caster_bonus_spontaneous)
	if levelUpVacian:
		if not char_editor.spells_check_complete(obj, class_extended_vancian):
			return 0
	if levelUpSpontaneous:
		if not char_editor.spells_check_complete(obj, class_extended_spontaneous):
			return 0
	return 1

def InitSpellSelection( obj , class_extended_vancian = 0, class_extended_spontaneous = 0, caster_bonus_vancian = 0, caster_bonus_spontaneous = 0):
	if class_extended_vancian <= 0 or class_extended_spontaneous <= 0:
		class_extended_vancian = char_class_utils.GetHighestVancianArcaneClass(obj)
		class_extended_spontaneous = char_class_utils.GetHighestSpontaneousArcaneClass(obj)
	levelUpVacian, levelUpSpontaneous = ClassNeedsSpellsOnLevelup(obj, class_extended_vancian, class_extended_spontaneous, caster_bonus_vancian, caster_bonus_spontaneous)
	if levelUpVacian:
		char_editor.init_spell_selection(obj, class_extended_vancian)
	if levelUpSpontaneous:
		char_editor.init_spell_selection(obj, class_extended_spontaneous)
	return 0
	
def LevelupSpellsFinalize( obj , class_extended_vancian = 0, class_extended_spontaneous = 0, caster_bonus_vancian = 0, caster_bonus_spontaneous = 0):
	if class_extended_vancian <= 0 or class_extended_spontaneous <= 0:
		class_extended_vancian = char_class_utils.GetHighestVancianArcaneClass(obj)
		class_extended_spontaneous = char_class_utils.GetHighestSpontaneousArcaneClass(obj)
	levelUpVacian, levelUpSpontaneous = ClassNeedsSpellsOnLevelup(obj, class_extended_vancian, class_extended_spontaneous, caster_bonus_vancian, caster_bonus_spontaneous)
	if levelUpVacian:
		caster_bonus_vancian = caster_bonus_vancian + 1
		char_editor.spells_finalize(obj, class_extended_vancian)
	if levelUpSpontaneous:
		caster_bonus_spontaneous = caster_bonus_spontaneous + 1
		char_editor.spells_finalize(obj, class_extended_spontaneous)
	return caster_bonus_vancian, caster_bonus_spontaneous
	