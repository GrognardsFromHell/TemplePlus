from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

###################################################

def GetConditionName():
	return "Mystic Theurge"

def GetSpellCasterConditionName():
	return "Mystic Theurge Spellcasting"
	
print "Registering " + GetConditionName()

classEnum = stat_level_mystic_theurge
classSpecModule = __import__('class030_mystic_theurge')

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

# Mystic Theurge raises the caster level for its two base classes specified in Modifier args 0 & 1

# configure the spell casting condition to hold the highest two Arcane/Divine classes as chosen-to-be-extended classes
def OnAddSpellCasting(attachee, args, evt_obj):
	#arg0 holds the arcane class
	if args.get_arg(0) == 0:
		args.set_arg(0, char_class_utils.GetHighestArcaneClass(attachee))
	
	#arg1 holds the divine class
	if args.get_arg(1) == 0:
		args.set_arg(1, char_class_utils.GetHighestDivineClass(attachee))
	return 0

def OnGetBaseCasterLevel(attachee, args, evt_obj):
	class_extended_1 = args.get_arg(0)
	class_extended_2 = args.get_arg(1)
	class_code = evt_obj.arg0
	if class_code != class_extended_1 and class_code != class_extended_2:
		if evt_obj.arg1 == 0: # are you specifically looking for the Mystic Theurge caster level?
			return 0
	classLvl = attachee.stat_level_get(classEnum)
	evt_obj.bonus_list.add(classLvl, 0, 137)
	return 0

def OnSpellListExtensionGet(attachee, args, evt_obj):
	class_extended_1 = args.get_arg(0)
	class_extended_2 = args.get_arg(1)
	class_code = evt_obj.arg0
	if (class_code != class_extended_1 and class_code != class_extended_2):
		if (evt_obj.arg1 == 0): # are you specifically looking for the Mystic Theurge caster level?
			return 0
	classLvl = attachee.stat_level_get(classEnum)
	evt_obj.bonus_list.add(classLvl, 0, 137)
	return 0

def OnInitLevelupSpellSelection(attachee, args, evt_obj):
	print "Mystic Theurge Spell Selection Init"
	if evt_obj.arg0 != classEnum:
		print str(evt_obj.arg0)
		print str(classEnum)
		return 0
	print "Mystic Theurge Spell Selection Init: Confirmed classEnum"
	class_extended_1 = args.get_arg(0)
	class_extended_2 = args.get_arg(1)
	print "Mystic Theurge Spell Selection: Class 1" + str(class_extended_1)
	print "Mystic Theurge Spell Selection: Class 2" + str(class_extended_2)
	classSpecModule.InitSpellSelection(attachee, class_extended_1, class_extended_2)
	return 0

def OnLevelupSpellsCheckComplete(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	class_extended_1 = args.get_arg(0)
	class_extended_2 = args.get_arg(1)
	if not classSpecModule.LevelupCheckSpells(attachee, class_extended_1, class_extended_2):
		evt_obj.bonus_list.add(-1, 0, 137) # denotes incomplete spell selection
	return 1

def OnLevelupSpellsFinalize(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	class_extended_1 = args.get_arg(0)
	class_extended_2 = args.get_arg(1)
	classSpecModule.LevelupSpellsFinalize(attachee, class_extended_1, class_extended_2)
	return

spellCasterSpecObj = PythonModifier(GetSpellCasterConditionName(), 8)
spellCasterSpecObj.AddHook(ET_OnConditionAdd, EK_NONE, OnAddSpellCasting, ())
spellCasterSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
spellCasterSpecObj.AddHook(ET_OnSpellListExtensionGet, EK_NONE, OnSpellListExtensionGet, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Activate, OnInitLevelupSpellSelection, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Check_Complete, OnLevelupSpellsCheckComplete, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())