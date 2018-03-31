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


def ElvenSaveBonusEnchantment(attachee, args, evt_obj):
	flags = evt_obj.flags
	if (flags & (2 << (D20STD_F_SPELL_SCHOOL_ENCHANTMENT-1))): 
		evt_obj.bonus_list.add(2, 31, 139) # Racial Bonus
	return 0


def OnGetSpellResistance(attachee, args, evt_obj):
	classLvl = attachee.stat_level_get(stat_level)
	evt_obj.bonus_list.add(11+classLvl, 36, "Racial Bonus (Drow)")
	return 0

def OnGetSpellResistanceTooltip(attachee, args, evt_obj):
	classLvl = attachee.stat_level_get(stat_level)
	evt_obj.append("Spell Resistance [" + str(11 + classLvl) + "]")
	return 0

def OnGetFavoredClass(attachee, args, evt_obj):
	gender = attachee.stat_level_get(stat_gender)
	if evt_obj.data1 == stat_level_cleric and gender == gender_female or  evt_obj.data1 == stat_level_wizard and gender == gender_male:
		evt_obj.return_val = 1
	return 0

def ConditionImmunityOnPreAdd(attachee, args, evt_obj):
	val = evt_obj.is_modifier("sp-Sleep")
	if val:
		evt_obj.return_val = 0
		attachee.float_mesfile_line( 'mes\\combat.mes', 5059, tf_red ) # "Sleep Immunity"
		game.create_history_from_pattern(31, attachee, OBJ_HANDLE_NULL)
	return 0


raceSpecObj = PythonModifier(GetConditionName(), 0)
race_utils.AddAbilityModifierHooks(raceSpecObj, raceSpecModule)
race_utils.AddSaveThrowBonusHook(raceSpecObj, D20_Save_Will, 2)
race_utils.AddSkillBonuses(raceSpecObj, {skill_listen: 2, skill_search: 2, skill_spot: 2})
race_utils.AddBaseMoveSpeed(raceSpecObj, 30)

raceSpecObj.AddHook(ET_OnGetSpellResistanceMod, EK_NONE, OnGetSpellResistance, ())
raceSpecObj.AddHook(ET_OnGetTooltip, EK_NONE, OnGetSpellResistanceTooltip, ())
raceSpecObj.AddHook(ET_OnSaveThrowLevel, EK_NONE, ElvenSaveBonusEnchantment, ())
raceSpecObj.AddHook(ET_OnD20Query, EK_Q_FavoredClass, OnGetFavoredClass, ())
raceSpecObj.AddHook(ET_OnConditionAddPre, EK_NONE, ConditionImmunityOnPreAdd, ())
#raceSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
