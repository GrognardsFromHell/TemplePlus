from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import race_utils

###################################################

def GetConditionName():
	return "Duergar"


raceEnum = race_dwarf + (3 << 5)
raceSpecModule = __import__('race097_duergar')
###################################################

BONUS_MES_RACIAL_BONUS = 139
BONUS_MES_STABILITY    = 317


def ElvenSaveBonusEnchantment(attachee, args, evt_obj):
	flags = evt_obj.flags
	if (flags & (2 << (D20STD_F_SPELL_SCHOOL_ENCHANTMENT-1))): 
		evt_obj.bonus_list.add(2, 31, BONUS_MES_RACIAL_BONUS) # Racial Bonus
	return 0


def OnGetFavoredClass(attachee, args, evt_obj):
	if evt_obj.data1 == stat_level_fighter:
		evt_obj.return_val = 1
	return 0

def ConditionImmunityOnPreAdd(attachee, args, evt_obj):
	val = evt_obj.is_modifier("Paralyzed")
	if val:
		evt_obj.return_val = 0
		attachee.float_text_line( "Paralysis Immunity", tf_red )
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

def OnGetAppraiseSkill(attachee, args, evt_obj):
	# adds appraise bonus to metal or rock items
	item = evt_obj.obj
	if item == OBJ_HANDLE_NULL:
		return 0
	item_material = item.obj_get_int(obj_f_material)
	if (item_material == mat_stone or item_material == mat_metal):
		evt_obj.bonus_list.add(2, 0, BONUS_MES_RACIAL_BONUS)
	return 0

def OnGetToHitBonusVsOrcsAndGoblins(attachee, args, evt_obj):
	target = evt_obj.attack_packet.target
	if target == OBJ_HANDLE_NULL:
		return 0
	if taget.is_cateogry_subtype(mc_subtype_half_orc) or target.is_cateogry_subtype(mc_subtype_goblinoid) or target.is_cateogry_subtype(mc_subtype_orc):
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


def CastInvisibility(attachee, args, evt_obj):
	radial_action = tpdp.RadialMenuEntryAction("Invisibility", D20A_CAST_SPELL, 0, "TAG_INTERFACE_HELP")
	spell_data = tpdp.D20SpellData(152)
	spell_data.set_spell_level(2)
	spell_data.spell_class = domain_special
	radial_action.set_spell_data(spell_data)
	radial_action.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0
##########################################################

raceSpecObj = PythonModifier(GetConditionName(), 0)
race_utils.AddAbilityModifierHooks(raceSpecObj, raceSpecModule)
race_utils.AddSaveThrowBonusHook(raceSpecObj, D20_Save_Will, 2)
race_utils.AddSaveBonusVsEffectType(raceSpecObj, D20STD_F_SPELL_LIKE_EFFECT, 2)
race_utils.AddSkillBonuses(raceSpecObj, {skill_listen: 1, skill_spot: 1, skill_move_silently: 4})
race_utils.AddBaseMoveSpeed(raceSpecObj, 20) # note: dwarven move speed with heavy armor or when medium/heavy encumbered is already handled in Encumbered Medium, Encumbered Heavy condition callbacks
race_utils.AddPoisonImmunity(raceSpecObj)
race_utils.AddFavoredClassHook(raceSpecObj, stat_level_fighter)

raceSpecObj.AddHook(ET_OnGetSkillLevel,   EK_SKILL_APPRAISE, OnGetAppraiseSkill, ())
raceSpecObj.AddHook(ET_OnGetMoveSpeed,    EK_NONE,           OnGetMoveSpeedSetLowerLimit, ())
raceSpecObj.AddHook(ET_OnSaveThrowLevel,  EK_NONE,           ElvenSaveBonusEnchantment, ())
raceSpecObj.AddHook(ET_OnConditionAddPre, EK_NONE,           ConditionImmunityOnPreAdd, ()) # paralysis immunity
raceSpecObj.AddHook(ET_OnToHitBonus2,     EK_NONE,           OnGetToHitBonusVsOrcsAndGoblins, ())
raceSpecObj.AddHook(ET_OnGetAC,           EK_NONE,           OnGetArmorClassBonusVsGiants, ())
raceSpecObj.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE,   OnAbilityModCheckStabilityBonus, ())
raceSpecObj.AddHook(ET_OnBuildRadialMenuEntry   , EK_NONE,   CastInvisibility, ())