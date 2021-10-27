from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

###################################################

def GetConditionName():
	return "Holy Scourge"

def GetSpellCasterConditionName():
	return "Holy Scourge Spellcasting"
	
print "Registering " + GetConditionName()

classEnum = stat_level_holy_scourge

###################################################


#### standard callbacks - BAB and Save values
def OnGetToHitBonusBase(attachee, args, evt_obj):
	classLvl = attachee.stat_level_get(classEnum)
	babvalue = game.get_bab_for_class(classEnum, classLvl)
	evt_obj.bonus_list.add(babvalue, 0, 137) # untyped, description: "Class"
	return 0

def OnGetSaveThrowFort(attachee, args, evt_obj):
	value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Fortitude)
	evt_obj.bonus_list.add(value, 0, 137)
	return 0

def OnGetSaveThrowReflex(attachee, args, evt_obj):
	value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Reflex)
	evt_obj.bonus_list.add(value, 0, 137)
	return 0

def OnGetSaveThrowWill(attachee, args, evt_obj):
	value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Will)
	evt_obj.bonus_list.add(value, 0, 137)
	return 0


classSpecObj = PythonModifier(GetConditionName(), 0)
classSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())

##### Spell casting

# configure the spell casting condition to hold the highest Arcane classs
def OnAddSpellCasting(attachee, args, evt_obj):
	#arg0 holds the arcane class
	if (args.get_arg(0) == 0):
		args.set_arg(0, char_class_utils.GetHighestArcaneClass(attachee))
	
	return 0

# Extend caster level for base casting class
def OnGetBaseCasterLevel(attachee, args, evt_obj):
	class_extended_1 = args.get_arg(0)
	class_code = evt_obj.arg0
	if (class_code != class_extended_1):
		if (evt_obj.arg1 == 0): # arg1 != 0 means you're looking for this particular class's contribution
			return 0
	classLvl = attachee.stat_level_get(classEnum)
	if classLvl > 1:
		evt_obj.bonus_list.add(classLvl - 1, 0, 137)
	return 0

def OnSpellListExtensionGet(attachee, args, evt_obj):
	class_extended_1 = args.get_arg(0)
	class_code = evt_obj.arg0
	if (class_code != class_extended_1):
		if (evt_obj.arg1 == 0): # arg1 != 0 means you're looking for this particular class's contribution
			return 0
	classLvl = attachee.stat_level_get(classEnum)
	if classLvl > 1:
		evt_obj.bonus_list.add(classLvl - 1, 0, 137)
	return 0

def OnInitLevelupSpellSelection(attachee, args, evt_obj):
	if (evt_obj.arg0 != classEnum):
		return 0
	classLvl = attachee.stat_level_get(classEnum)
	if (classLvl == 0):
		return 0
	class_extended_1 = args.get_arg(0)
	classSpecModule.InitSpellSelection(attachee, class_extended_1)
	return 0

def OnLevelupSpellsCheckComplete(attachee, args, evt_obj):
	if (evt_obj.arg0 != classEnum):
		return 0
	class_extended_1 = args.get_arg(0)
	if (not classSpecModule.LevelupCheckSpells(attachee, class_extended_1) ):
		evt_obj.bonus_list.add(-1, 0, 137) # denotes incomplete spell selection
	return 1
	
def OnLevelupSpellsFinalize(attachee, args, evt_obj):
	if (evt_obj.arg0 != classEnum):
		return 0
	classLvl = attachee.stat_level_get(classEnum)
	if (classLvl == 0):
		return 0
	class_extended_1 = args.get_arg(0)
	classSpecModule.LevelupSpellsFinalize(attachee, class_extended_1)
	return

spellCasterSpecObj = PythonModifier(GetSpellCasterConditionName(), 8)
spellCasterSpecObj.AddHook(ET_OnConditionAdd, EK_NONE, OnAddSpellCasting, ())
spellCasterSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
spellCasterSpecObj.AddHook(ET_OnSpellListExtensionGet, EK_NONE, OnSpellListExtensionGet, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Activate, OnInitLevelupSpellSelection, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Check_Complete, OnLevelupSpellsCheckComplete, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())

dedicatedSpellcaster = PythonModifier("Dedicated Spellcaster", 2) #Spare, Spare
dedicatedSpellcaster.MapToFeat("Dedicated Spellcaster")

righteousEvocation = PythonModifier("Righteous Evocation", 2) #Spare, Spare
righteousEvocation.MapToFeat("Righteous Evocation")

arcaneSmite = PythonModifier("Arcane Smite", 2) #Spare, Spare
arcaneSmite.MapToFeat("Arcane Smite ")


def DevotedArcanistCasterBonus(attachee, args, evt_obj):
	#Check target alignment
	align = evt_obj.target.stat_level_get(stat_alignment)
	if align & ALIGNMENT_EVIL:
		classLvl = attachee.stat_level_get(classEnum)
		evt_obj.bonus_list.add(classLvl, 0, "Devoted Arcanist")	
	return 0

devotedArcanist = PythonModifier("Devoted Arcanist", 2) #Spare, Spare
devotedArcanist.MapToFeat("Devoted Arcanist")
devotedArcanist.AddHook(ET_OnSpellResistanceCheckBonus, EK_NONE, DevotedArcanistCasterBonus, ())

