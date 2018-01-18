from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

###################################################

def GetConditionName():
	return "Rogue"

print "Registering " + GetConditionName()

classEnum = stat_level_rogue
classSpecModule = __import__('class015_rogue')

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


def RogueSneakAttackDice(attachee, args, evt_obj):
	rogLvl = attachee.stat_level_get(classEnum)
	if rogLvl <= 0:
		return 0
	evt_obj.return_val += 1+ (rogLvl - 1)/2
	return 0

classSpecExtender = PythonModifier()
classSpecExtender.ExtendExisting("Rogue")
classSpecExtender.AddHook(ET_OnD20PythonQuery, "Sneak Attack Dice", RogueSneakAttackDice, ())