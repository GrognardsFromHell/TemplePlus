from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import race_utils

###################################################

def GetConditionName():
	return "Gray Elf"


raceEnum = race_elf + (3 << 5)
raceSpecModule = __import__('race098_gray_elf')
###################################################


def ElvenSaveBonusEnchantment(attachee, args, evt_obj):
	flags = evt_obj.flags
	if (flags & (2 << (D20STD_F_SPELL_SCHOOL_ENCHANTMENT-1))): 
		evt_obj.bonus_list.add(2, 31, 139) # Racial Bonus
	return 0


def OnGetFavoredClass(attachee, args, evt_obj):
	if evt_obj.data1 == stat_level_wizard:
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
race_utils.AddSkillBonuses(raceSpecObj, {skill_listen: 2, skill_search: 2, skill_spot: 2})
race_utils.AddBaseMoveSpeed(raceSpecObj, 30)

raceSpecObj.AddHook(ET_OnSaveThrowLevel, EK_NONE, ElvenSaveBonusEnchantment, ())
raceSpecObj.AddHook(ET_OnD20Query, EK_Q_FavoredClass, OnGetFavoredClass, ())
raceSpecObj.AddHook(ET_OnConditionAddPre, EK_NONE, ConditionImmunityOnPreAdd, ())

