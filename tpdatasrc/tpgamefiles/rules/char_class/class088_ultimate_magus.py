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
	bonFeatInfo = tpdp.get_metamagic_feats()
	char_editor.set_bonus_feats(bonFeatInfo)
	return
	
#Check if which levels get increased.  Returns a flag for vacian and natural.
def CheckUMIncreaseOnLevelup(umLevel, naturalCasterLevel, vacianCasterLevel):
	levelUpVacian = true
	levelUpNatural = true
	if level in [1,4,7]:
		if naturalLvl2 < vacianLvl1:
			levelUpVacian = false
		else:
			levelUpNatural = false
	return levelUpVacian, levelUpNatural
	
#Check if the class gets spells on this levelup.  Returns a flag for vacian and natural
def ClassNeedsSpellsOnLevelup(obj, class_extended_1, class_extended_2):
	newLvl = obj.stat_level_get( classEnum ) + 1
	vacianLvl1, naturalLvl1= GetUMCasterLevels(obj, class_extended_1, class_extended_2)
	return CheckUMLevelIncrease(newLvl, vacianLvl1, naturalLvl1)

def IsSelectingSpellsOnLevelup( obj , class_extended_1 = 0, class_extended_2 = 0, caster_bonus_1 = 0, caster_bonus_2 = 0):
	if class_extended_1 <= 0 or class_extended_2 <= 0:
		class_extended_1 = char_class_utils.GetHighestVancianArcaneClass(obj)
		class_extended_2 = char_class_utils.GetHighestSpontaneousArcaneClass(obj)
	levelUpVacian, levelUpNatural = ClassNeedsSpellsOnLevelup(obj, class_extended_1, class_extended_2, caster_bonus_1, caster_bonus_2)
	if levelUpVacian:
		if char_editor.is_selecting_spells(obj, class_extended_1):
			return 1
	if levelUpNatural:
		if char_editor.is_selecting_spells(obj, class_extended_2):
			return 1
	return 0

def LevelupCheckSpells( obj , class_extended_1 = 0, class_extended_2 = 0, caster_bonus_1 = 0, caster_bonus_2 = 0):
	if class_extended_1 <= 0 or class_extended_2 <= 0:
		class_extended_1 = char_class_utils.GetHighestVancianArcaneClass(obj)
		class_extended_2 = char_class_utils.GetHighestSpontaneousArcaneClass(obj)
	levelUpVacian, levelUpNatural = ClassNeedsSpellsOnLevelup(obj, class_extended_1, class_extended_2, caster_bonus_1, caster_bonus_2)
	if levelUpVacian:
		if not char_editor.spells_check_complete(obj, class_extended_1):
			return 0
	if levelUpNatural:
		if not char_editor.spells_check_complete(obj, class_extended_2):
			return 0
	return 1

def InitSpellSelection( obj , class_extended_1 = 0, class_extended_2 = 0, caster_bonus_1 = 0, caster_bonus_2 = 0):
	if class_extended_1 <= 0 or class_extended_2 <= 0:
		class_extended_1 = char_class_utils.GetHighestVancianArcaneClass(obj)
		class_extended_2 = char_class_utils.GetHighestSpontaneousArcaneClass(obj)
	levelUpVacian, levelUpNatural = ClassNeedsSpellsOnLevelup(obj, class_extended_1, class_extended_2, caster_bonus_1, caster_bonus_2)
	if levelUpVacian:
		char_editor.init_spell_selection(obj, class_extended_1)
	if levelUpNatural:
		char_editor.init_spell_selection(obj, class_extended_2)
	return 0
	
def LevelupSpellsFinalize( obj , class_extended_1 = 0, class_extended_2 = 0, caster_bonus_1 = 0, caster_bonus_2 = 0):
	if class_extended_1 <= 0 or class_extended_2 <= 0:
		class_extended_1 = char_class_utils.GetHighestVancianArcaneClass(obj)
		class_extended_2 = char_class_utils.GetHighestSpontaneousArcaneClass(obj)
	levelUpVacian, levelUpNatural = ClassNeedsSpellsOnLevelup(obj, class_extended_1, class_extended_2, caster_bonus_1, caster_bonus_2)
	if levelUpVacian:
		caster_bonus_1 = caster_bonus_1 + 1
		char_editor.spells_finalize(obj, class_extended_1)
	if levelUpNatural:
		caster_bonus_2 = caster_bonus_2 + 1
		char_editor.spells_finalize(obj, class_extended_2)
	return caster_bonus_1, caster_bonus_2
	