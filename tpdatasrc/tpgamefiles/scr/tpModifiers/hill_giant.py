from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import race_utils

###################################################

def GetConditionName():
	return "Hill Giant"


raceEnum = race_hill_giant
raceSpecModule = __import__('race010_hill_giant')
###################################################


#### standard callbacks - BAB and Save values
def OnGetToHitBonusBase(attachee, args, evt_obj):
	classLvl = attachee.stat_level_get(classEnum)
	babvalue = game.get_bab_for_class(classEnum, classLvl)
	evt_obj.bonus_list.add(babvalue, 0, 137) # untyped, description: "Class"
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
race_utils.AddAbilityModifierHooks(raceSpecObj, raceSpecModule)
race_utils.AddFavoredClassHook(raceSpecObj, stat_level_barbarian)
race_utils.AddSaveThrowBonusHook(raceSpecObj, D20_Save_Fortitude, 8)
race_utils.AddSaveThrowBonusHook(raceSpecObj, D20_Save_Will, 4)
race_utils.AddSaveThrowBonusHook(raceSpecObj, D20_Save_Reflex, 4)
race_utils.AddBaseMoveSpeed(raceSpecObj, 30)
