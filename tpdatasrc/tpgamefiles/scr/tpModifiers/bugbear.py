from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import race_utils

###################################################

def GetConditionName():
	return "Bugbear"


raceEnum = race_bugbear
raceSpecModule = __import__('race008_bugbear')
###################################################


raceSpecObj = PythonModifier(GetConditionName(), 0)
race_utils.AddAbilityModifierHooks(raceSpecObj, raceSpecModule)
race_utils.AddFavoredClassHook(raceSpecObj, stat_level_rogue)
race_utils.AddSaveThrowBonusHook(raceSpecObj, D20_Save_Fortitude, 1)
race_utils.AddSaveThrowBonusHook(raceSpecObj, D20_Save_Will, 1)
race_utils.AddSaveThrowBonusHook(raceSpecObj, D20_Save_Reflex, 3)
race_utils.AddSkillBonuses(raceSpecObj, {skill_move_silently: 4})
race_utils.AddBaseMoveSpeed(raceSpecObj, 30)


