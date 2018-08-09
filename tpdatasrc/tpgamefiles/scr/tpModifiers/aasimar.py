from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import race_utils

###################################################

def GetConditionName():
	return "Aasimar"


raceEnum = race_human + (2 << 5)
raceSpecModule = __import__('race064_aasimar')
###################################################



raceSpecObj = PythonModifier(GetConditionName(), 0)
race_utils.AddAbilityModifierHooks(raceSpecObj, raceSpecModule)
race_utils.AddSkillBonuses(raceSpecObj, {skill_listen: 2, skill_spot: 2})
race_utils.AddBaseMoveSpeed(raceSpecObj, 30)
race_utils.AddFavoredClassHook(raceSpecObj, stat_level_paladin)
race_utils.AddDamageResistances(raceSpecObj, {D20DT_ACID: 5, D20DT_COLD: 5, D20DT_ELECTRICITY: 5})
