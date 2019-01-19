from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import race_utils

###################################################

def GetConditionName():
	return "Gold Dwarf"


raceEnum = race_dwarf + (5 << 5)
raceSpecModule = __import__('race161_gold_dwarf')
###################################################

BONUS_MES_RACIAL_BONUS = 139
BONUS_MES_STABILITY    = 317

def DwarfSaveBonus(attachee, args, evt_obj):
	flags = evt_obj.flags
	if (flags & (1 << (D20STD_F_SPELL_LIKE_EFFECT-1))): 
		evt_obj.bonus_list.add(2, 31, BONUS_MES_RACIAL_BONUS) # Racial Bonus
	elif (flags & (1 << (D20STD_F_POISON-1))): 
		evt_obj.bonus_list.add(2, 31, BONUS_MES_RACIAL_BONUS) # Racial Bonus
	return 0

def OnGetFavoredClass(attachee, args, evt_obj):
	if evt_obj.data1 == stat_level_fighter:
		evt_obj.return_val = 1
	return 0

def OnGetMoveSpeedSetLowerLimit(attachee, args, evt_obj):
	# this sets the lower limit for dwarf move speed at 20, unless someone has already set it (e.g. by web/entangle)
	if evt_obj.bonus_list.flags & 2:
		return 0
	moveSpeedCapValue = 20
	capFlags = 2 # set lower limit
	capType  = 0 # operate on all bonus types
	bonusMesline = BONUS_MES_RACIAL_BONUS # racial ability
	evt_obj.bonus_list.set_overall_cap(capFlags, moveSpeedCapValue, capType, bonusMesline)
	return 0

def OnGetToHitBonusVsAberration(attachee, args, evt_obj):
	target = evt_obj.attack_packet.target
	if target == OBJ_HANDLE_NULL:
		return 0
	
	if target.is_category_type(mc_type_aberration):
		evt_obj.bonus_list.add(1, 0, BONUS_MES_RACIAL_BONUS)
	return 0

def OnGetArmorClassBonusVsGiants(attachee, args, evt_obj):
	attacker = evt_obj.attack_packet.attacker
	if attacker == OBJ_HANDLE_NULL:
		return 0
	if attacker.is_category_type(mc_type_giant):
		evt_obj.bonus_list.add(4, 8, BONUS_MES_RACIAL_BONUS)
	return 0

def OnAbilityModCheckStabilityBonus(attachee, args, evt_obj):
	flags = evt_obj.flags
	if (flags & 1) and (flags & 2): # defender bonus
		evt_obj.bonus_list.add(4, 22, BONUS_MES_STABILITY)
	return 0

##########################################################

raceSpecObj = PythonModifier(GetConditionName(), 0)
race_utils.AddAbilityModifierHooks(raceSpecObj, raceSpecModule)

race_utils.AddBaseMoveSpeed(raceSpecObj, 20) # note: dwarven move speed with heavy armor or when medium/heavy encumbered is already handled in Encumbered Medium, Encumbered Heavy condition callbacks
race_utils.AddFavoredClassHook(raceSpecObj, stat_level_fighter)

raceSpecObj.AddHook(ET_OnGetMoveSpeed,    EK_NONE,           OnGetMoveSpeedSetLowerLimit, ())
raceSpecObj.AddHook(ET_OnToHitBonus2,     EK_NONE,           OnGetToHitBonusVsAberration, ())
raceSpecObj.AddHook(ET_OnGetAC,           EK_NONE,           OnGetArmorClassBonusVsGiants, ())
raceSpecObj.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE,   OnAbilityModCheckStabilityBonus, ())
raceSpecObj.AddHook(ET_OnSaveThrowLevel, EK_NONE,   DwarfSaveBonus, ())

