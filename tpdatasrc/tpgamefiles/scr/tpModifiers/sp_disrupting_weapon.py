from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from spell_utils import verifyItem

# Object spell effect
# args:
#		0: spell_id
#		1: duration
#		2: dc
#		3: extra
#		4: extra

print "Registering sp_disrupting_weapon"

def AddChar(weapon, args, evt_obj):
	spell_id = args.get_arg(0)
	duration = args.get_arg(1)
	dc = args.get_arg(2)

	packet = tpdp.SpellPacket(spell_id)
	clvl = packet.caster_level

	weapon.item_condition_add_with_args(
			'Disrupting Weapon', dc, clvl, 0, 0, spell_id)
	parent = weapon.obj_get_obj(obj_f_item_parent)

	return 0

def OnEnd(weapon, args, evt_obj):
	weapon.item_condition_remove('Disrupting Weapon', args.get_arg(0))
	return 0

wpn = PythonModifier('sp-Disrupting Weapon', 5, 1)
# Note: no tooltip because it doesn't seem to matter
wpn.AddHook(ET_OnConditionAdd, EK_NONE, AddChar, ())
wpn.AddHook(ET_OnConditionRemove, EK_NONE, OnEnd, ())

wpn.AddSpellDispelCheckStandard()
wpn.AddSpellTeleportPrepareStandard()
wpn.AddSpellTeleportReconnectStandard()
wpn.AddSpellCountdownStandardHook()

# Character Item Effect
# args
#  0: dc
#  1: hit dice
#  2: weapon location #
#  3: spare
#  4: spell id

def Disrupt(attacker, args, evt_obj):
	weapon = evt_obj.attack_packet.get_weapon_used()
	target = evt_obj.attack_packet.target

	dc = args.get_arg(0)
	hd = args.get_arg(1)

	if not verifyItem(weapon, args): return 0
	if not target.is_category_type(mc_type_undead): return 0
	if target.hit_dice_num > hd: return 0
	if target.saving_throw(dc, D20_Save_Will, D20STD_F_NONE, weapon):
		return 0

	# kill happens before damage
	if target.type == obj_t_npc:
		target.obj_set_obj(obj_f_last_hit_by, attacker)

	game.particles('sp-Destroy Undead', target)
	target.critter_kill()
	msg = game.get_mesline('mes\\combat.mes', 7000)
	game.create_history_freeform("{}\n\n".format(msg))

	return 0

def Glow(char, args, evt_obj):
	if verifyItem(evt_obj.get_obj_from_args(),args):
		if evt_obj.return_val < 5:
			evt_obj.return_val = 5

	return 0

cha = PythonModifier('Disrupting Weapon', 5, 0)
cha.AddHook(ET_OnDealingDamage2, EK_NONE, Disrupt, ())
cha.AddHook(ET_OnWeaponGlowType, EK_NONE, Glow, ())
