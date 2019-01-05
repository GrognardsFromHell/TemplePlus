from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import race_utils

###################################################

def GetConditionName():
	return "Svirfneblin"


raceEnum = race_gnome + (1 << 5)
raceSpecModule = __import__('race035_svirfneblin')
###################################################


def OnGetSpellResistance(attachee, args, evt_obj):
	classLvl = attachee.stat_level_get(stat_level)
	evt_obj.bonus_list.add(11+classLvl, 36, "Racial Bonus (Svirfneblin)")
	return 0

def OnGetSpellResistanceTooltip(attachee, args, evt_obj):
	classLvl = attachee.stat_level_get(stat_level)
	evt_obj.append("Spell Resistance [" + str(11 + classLvl) + "]")
	return 0

def OnGetFavoredClass(attachee, args, evt_obj):
	if evt_obj.data1 == stat_level_rogue:
		evt_obj.return_val = 1
	return 0


def OnGetToHitBonusVsOrcsAndKobolds(attachee, args, evt_obj):
	target = evt_obj.attack_packet.target
	if target == OBJ_HANDLE_NULL:
		return 0
	if target.is_category_subtype(mc_subtype_goblinoid):
		evt_obj.bonus_list.add(1, 0, 139) # Racial Bonus
	return 0
	
def OnGetDodgeBonus(attachee, args, evt_obj):
	evt_obj.bonus_list.add(4, 0, 139) # Racial Bonus
	return 0

def OnIllusionDCBonus(attachee, args, evt_obj):
	school = evt_obj.spell_entry.spell_school_enum
	if school == Illusion:
		evt_obj.bonus_list.add(1, 31, 139) # Racial Bonus
	return 0
	

raceSpecObj = PythonModifier(GetConditionName(), 0)
race_utils.AddAbilityModifierHooks(raceSpecObj, raceSpecModule)
race_utils.AddSaveThrowBonusHook(raceSpecObj, D20_Save_Will, 2)
race_utils.AddSkillBonuses(raceSpecObj, {skill_hide: 2, skill_listen: 2})
race_utils.AddBaseMoveSpeed(raceSpecObj, 20)

raceSpecObj.AddHook(ET_OnGetSpellResistanceMod, EK_NONE, OnGetSpellResistance, ())
raceSpecObj.AddHook(ET_OnGetTooltip, EK_NONE, OnGetSpellResistanceTooltip, ())
raceSpecObj.AddHook(ET_OnD20Query, EK_Q_FavoredClass, OnGetFavoredClass, ())
raceSpecObj.AddHook(ET_OnToHitBonus2, EK_NONE, OnGetToHitBonusVsOrcsAndKobolds, ())
raceSpecObj.AddHook(ET_OnGetAC, EK_NONE, OnGetDodgeBonus, ())
raceSpecObj.AddHook(ET_OnGetSpellDcMod, EK_NONE, OnIllusionDCBonus, ())
