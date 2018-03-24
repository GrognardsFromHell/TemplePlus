from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import race_utils

###################################################

def GetConditionName():
	return "Drow"


raceEnum = race_elf + (2 << 5)
raceSpecModule = __import__('race066_drow')
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

def OnGetSpellResistance(attachee, args, evt_obj):
	classLvl = attachee.stat_level_get(stat_level)
	evt_obj.bonus_list.add(11+classLvl, 36, "Racial Bonus (Drow)")
	return 0

def OnGetSpellResistanceTooltip(attachee, args, evt_obj):
	classLvl = attachee.stat_level_get(stat_level)
	evt_obj.append("Spell Resistance [" + str(11 + classLvl) + "]")
	return 0


raceSpecObj = PythonModifier(GetConditionName(), 0)
raceSpecObj.AddHook(ET_OnGetSpellResistanceMod, EK_NONE, OnGetSpellResistance, ())
raceSpecObj.AddHook(ET_OnGetTooltip, EK_NONE, OnGetSpellResistanceTooltip, ())
#raceSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
#raceSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
#raceSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
#raceSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())
