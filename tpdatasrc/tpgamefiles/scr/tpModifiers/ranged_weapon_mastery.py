from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Ranged Weapon Mastery"

bon_type_rwm = 856 #Using the same type as melee weapon mastery since those bonuses should not be together
bon_val = 2

def rwmToHit(attachee, args, evt_obj):
	featDamType = args.get_param(0)
	wpn = evt_obj.attack_packet.get_weapon_used()
	if wpn == OBJ_HANDLE_NULL:
		return 0

	weapDamType = wpn.obj_get_int(obj_f_weapon_attacktype)
	
	#Must be a ranged attack
	if not evt_obj.attack_packet.get_flags() & D20CAF_RANGED:
		return 0

	if weapDamType == featDamType or weapDamType == D20DT_SLASHING_AND_BLUDGEONING_AND_PIERCING:
		evt_obj.bonus_list.add_from_feat(bon_val, bon_type_rwm, 114, "Ranged Weapon Mastery")
		return 0
	elif featDamType == D20DT_BLUDGEONING and (weapDamType == D20DT_BLUDGEONING_AND_PIERCING or weapDamType == D20DT_SLASHING_AND_BLUDGEONING):
		evt_obj.bonus_list.add_from_feat(bon_val, bon_type_rwm, 114, "Ranged Weapon Mastery")
		return 0
	elif featDamType == D20DT_SLASHING and (weapDamType == D20DT_SLASHING_AND_BLUDGEONING or weapDamType == D20DT_PIERCING_AND_SLASHING):
		evt_obj.bonus_list.add_from_feat(bon_val, bon_type_rwm, 114, "Ranged Weapon Mastery")
		return 0
	elif featDamType == D20DT_PIERCING and (weapDamType == D20DT_BLUDGEONING_AND_PIERCING or weapDamType == D20DT_PIERCING_AND_SLASHING):
		evt_obj.bonus_list.add_from_feat(bon_val, bon_type_rwm, 114, "Ranged Weapon Mastery")
		return 0
	return 0


def rwmToDam(attachee, args, evt_obj):
	featDamType = args.get_param(0)
	wpn = evt_obj.attack_packet.get_weapon_used()
	if wpn == OBJ_HANDLE_NULL:
		return 0
	
	weapDamType = wpn.obj_get_int(obj_f_weapon_attacktype)
	
	#Must be a ranged attack
	if not evt_obj.attack_packet.get_flags() & D20CAF_RANGED:
		return 0

	if weapDamType == featDamType or weapDamType == D20DT_SLASHING_AND_BLUDGEONING_AND_PIERCING:
		evt_obj.damage_packet.bonus_list.add_from_feat(bon_val, bon_type_rwm, 114, "Ranged Weapon Mastery")
		return 0
	elif featDamType == D20DT_BLUDGEONING and (weapDamType == D20DT_BLUDGEONING_AND_PIERCING or weapDamType == D20DT_SLASHING_AND_BLUDGEONING):
		evt_obj.damage_packet.bonus_list.add_from_feat(bon_val, bon_type_rwm, 114, "Ranged Weapon Mastery")
		return 0
	elif featDamType == D20DT_SLASHING and (weapDamType == D20DT_SLASHING_AND_BLUDGEONING or weapDamType == D20DT_PIERCING_AND_SLASHING):
		evt_obj.damage_packet.bonus_list.add_from_feat(bon_val, bon_type_rwm, 114, "Ranged Weapon Mastery")
		return 0
	elif featDamType == D20DT_PIERCING and (weapDamType == D20DT_BLUDGEONING_AND_PIERCING or weapDamType == D20DT_PIERCING_AND_SLASHING):
		evt_obj.damage_packet.bonus_list.add_from_feat(bon_val, bon_type_rwm, 114, "Ranged Weapon Mastery")
		return 0
	return 0
	
	
def rwmRangeIncrementBonus(attachee, args, evt_obj):
	featDamType = args.get_param(0)
	wpn = evt_obj.weapon_used
	
	if wpn == OBJ_HANDLE_NULL:
		return 0
		
	weapDamType = wpn.obj_get_int(obj_f_weapon_attacktype)
	
	#Add 20 feet to the weapon range
	if weapDamType == featDamType or weapDamType == D20DT_SLASHING_AND_BLUDGEONING_AND_PIERCING:
		evt_obj.range_bonus = evt_obj.range_bonus + 20
	elif featDamType == D20DT_BLUDGEONING and (weapDamType == D20DT_BLUDGEONING_AND_PIERCING or weapDamType == D20DT_SLASHING_AND_BLUDGEONING):
		evt_obj.range_bonus = evt_obj.range_bonus + 20
	elif featDamType == D20DT_SLASHING and (weapDamType == D20DT_SLASHING_AND_BLUDGEONING or weapDamType == D20DT_PIERCING_AND_SLASHING):
		evt_obj.range_bonus = evt_obj.range_bonus + 20
	elif featDamType == D20DT_PIERCING and (weapDamType == D20DT_BLUDGEONING_AND_PIERCING or weapDamType == D20DT_PIERCING_AND_SLASHING):
		evt_obj.range_bonus = evt_obj.range_bonus + 20
	return 0

rwmBludg = PythonModifier("Ranged Weapon Mastery - Bludgeoning", 3)
rwmBludg.MapToFeat("Ranged Weapon Mastery - Bludgeoning")
rwmBludg.AddHook(ET_OnToHitBonus2, EK_NONE, rwmToHit, (D20DT_BLUDGEONING,))
rwmBludg.AddHook(ET_OnDealingDamage, EK_NONE, rwmToDam, (D20DT_BLUDGEONING,))
rwmBludg.AddHook(ET_OnRangeIncrementBonus, EK_NONE, rwmRangeIncrementBonus, (D20DT_BLUDGEONING,))

rwmSlash= PythonModifier("Ranged Weapon Mastery - Slashing", 3)
rwmSlash.MapToFeat("Ranged Weapon Mastery - Slashing")
rwmSlash.AddHook(ET_OnToHitBonus2, EK_NONE, rwmToHit, (D20DT_SLASHING,))
rwmSlash.AddHook(ET_OnDealingDamage, EK_NONE, rwmToDam, (D20DT_SLASHING,))
rwmSlash.AddHook(ET_OnRangeIncrementBonus, EK_NONE, rwmRangeIncrementBonus, (D20DT_SLASHING,))

rwmPierc = PythonModifier("Ranged Weapon Mastery - Piercing", 3)
rwmPierc.MapToFeat("Ranged Weapon Mastery - Piercing")
rwmPierc.AddHook(ET_OnToHitBonus2, EK_NONE, rwmToHit, (D20DT_PIERCING,))
rwmPierc.AddHook(ET_OnDealingDamage, EK_NONE, rwmToDam, (D20DT_PIERCING,))
rwmPierc.AddHook(ET_OnRangeIncrementBonus, EK_NONE, rwmRangeIncrementBonus, (D20DT_PIERCING,))

