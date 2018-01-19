from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Melee Weapon Mastery"

bon_type_mwm = 856
bon_val = 2

def MWMToHit(attachee, args, evt_obj):
	featDamType = args.get_param(0)
	wpn = evt_obj.attack_packet.get_weapon_used()
	if wpn == OBJ_HANDLE_NULL:
		if not (attachee.has_feat(feat_improved_unarmed_strike)):
			return 0
		weapDamType = D20DT_BLUDGEONING
	else:
		weapDamType = wpn.obj_get_int(obj_f_weapon_attacktype)
	if evt_obj.attack_packet.get_flags() & D20CAF_RANGED:
		return 0

	if weapDamType == featDamType or weapDamType == D20DT_SLASHING_AND_BLUDGEONING_AND_PIERCING:
		evt_obj.bonus_list.add_from_feat(bon_val, bon_type_mwm, 114, "Melee Weapon Mastery")
		return 0
	elif featDamType == D20DT_BLUDGEONING and (weapDamType == D20DT_BLUDGEONING_AND_PIERCING or weapDamType == D20DT_SLASHING_AND_BLUDGEONING):
		evt_obj.bonus_list.add_from_feat(bon_val, bon_type_mwm, 114, "Melee Weapon Mastery")
		return 0
	elif featDamType == D20DT_SLASHING and (weapDamType == D20DT_SLASHING_AND_BLUDGEONING or weapDamType == D20DT_PIERCING_AND_SLASHING):
		evt_obj.bonus_list.add_from_feat(bon_val, bon_type_mwm, 114, "Melee Weapon Mastery")
		return 0
	elif featDamType == D20DT_PIERCING and (weapDamType == D20DT_BLUDGEONING_AND_PIERCING or weapDamType == D20DT_PIERCING_AND_SLASHING):
		evt_obj.bonus_list.add_from_feat(bon_val, bon_type_mwm, 114, "Melee Weapon Mastery")
		return 0
	return 0


def MWMToDam(attachee, args, evt_obj):
	featDamType = args.get_param(0)
	wpn = evt_obj.attack_packet.get_weapon_used()
	if wpn == OBJ_HANDLE_NULL:
		if not (attachee.has_feat(feat_improved_unarmed_strike)):
			return 0
		weapDamType = D20DT_BLUDGEONING
	else:
		weapDamType = wpn.obj_get_int(obj_f_weapon_attacktype)
	if evt_obj.attack_packet.get_flags() & D20CAF_RANGED:
		return 0


	if weapDamType == featDamType or weapDamType == D20DT_SLASHING_AND_BLUDGEONING_AND_PIERCING:
		evt_obj.damage_packet.bonus_list.add_from_feat(bon_val, bon_type_mwm, 114, "Melee Weapon Mastery")
		return 0
	elif featDamType == D20DT_BLUDGEONING and (weapDamType == D20DT_BLUDGEONING_AND_PIERCING or weapDamType == D20DT_SLASHING_AND_BLUDGEONING):
		evt_obj.damage_packet.bonus_list.add_from_feat(bon_val, bon_type_mwm, 114, "Melee Weapon Mastery")
		return 0
	elif featDamType == D20DT_SLASHING and (weapDamType == D20DT_SLASHING_AND_BLUDGEONING or weapDamType == D20DT_PIERCING_AND_SLASHING):
		evt_obj.damage_packet.bonus_list.add_from_feat(bon_val, bon_type_mwm, 114, "Melee Weapon Mastery")
		return 0
	elif featDamType == D20DT_PIERCING and (weapDamType == D20DT_BLUDGEONING_AND_PIERCING or weapDamType == D20DT_PIERCING_AND_SLASHING):
		evt_obj.damage_packet.bonus_list.add_from_feat(bon_val, bon_type_mwm, 114, "Melee Weapon Mastery")
		return 0
	return 0

mwmBludg = PythonModifier("Melee Weapon Mastery - Bludgeoning", 3)
mwmBludg.MapToFeat("Melee Weapon Mastery - Bludgeoning")
mwmBludg.AddHook(ET_OnToHitBonus2, EK_NONE, MWMToHit, (D20DT_BLUDGEONING,))
mwmBludg.AddHook(ET_OnDealingDamage, EK_NONE, MWMToDam, (D20DT_BLUDGEONING,))

mwmSlash= PythonModifier("Melee Weapon Mastery - Slashing", 3)
mwmSlash.MapToFeat("Melee Weapon Mastery - Slashing")
mwmSlash.AddHook(ET_OnToHitBonus2, EK_NONE, MWMToHit, (D20DT_SLASHING,))
mwmSlash.AddHook(ET_OnDealingDamage, EK_NONE, MWMToDam, (D20DT_SLASHING,))

mwmPierc = PythonModifier("Melee Weapon Mastery - Piercing", 3)
mwmPierc.MapToFeat("Melee Weapon Mastery - Piercing")
mwmPierc.AddHook(ET_OnToHitBonus2, EK_NONE, MWMToHit, (D20DT_PIERCING,))
mwmPierc.AddHook(ET_OnDealingDamage, EK_NONE, MWMToDam, (D20DT_PIERCING,))


# MWM bonuses do not stack for weapons with multiple damage types. See:
# http://rpg.stackexchange.com/questions/57989/can-you-take-melee-weapon-mastery-twice-and-have-it-apply-twice-to-the-same-weap