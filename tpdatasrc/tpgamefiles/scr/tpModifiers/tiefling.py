from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import race_utils

###################################################

def GetConditionName():
	return "Tiefling"


raceEnum = race_human + (3 << 5)
raceSpecModule = __import__('race096_tiefling')
###################################################



raceSpecObj = PythonModifier(GetConditionName(), 0)
race_utils.AddAbilityModifierHooks(raceSpecObj, raceSpecModule)
race_utils.AddSkillBonuses(raceSpecObj, {skill_bluff: 2, skill_hide: 2})
race_utils.AddBaseMoveSpeed(raceSpecObj, 30)
race_utils.AddFavoredClassHook(raceSpecObj, stat_level_rogue)
race_utils.AddDamageResistances(raceSpecObj, {D20DT_FIRE: 5, D20DT_COLD: 5, D20DT_ELECTRICITY: 5})
