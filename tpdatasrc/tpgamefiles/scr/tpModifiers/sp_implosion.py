from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions
from utilities import *
from spell_utils import *

implodeEnum = 901

# Seems rather unnecessary since spell casting already removes
# concentration, but the original has it.
def PreAdd(char, args, evt_obj):
	if evt_obj.is_modifier('sp-Concentrating'):
		args.remove_spell()
		args.remove_spell_mod()
	return 0

def Yes(char, args, evt_obj):
	evt_obj.return_val = 1
	return 0

def IsConcentrating(char, args, evt_obj):
	evt_obj.data1 = args.get_arg(0)
	evt_obj.data2 = 0
	evt_obj.return_val = 1
	return 0

def DmgCheck(char, args, evt_obj):
	spell_id = args.get_arg(0)
	spacket = tpdp.SpellPacket(spell_id)
	dpacket = evt_obj.damage_packet
	# Note: this could be wrong for spells <9th level, but it's tricky to
	# calculate the right level
	dc = 10 + spacket.spell_known_slot_level + dpacket.final_damage

	if not char.skill_roll(skill_concentration, dc, 0):
		char.float_mesfile_line('mes\\combat.mes', 5060)
		char.d20_send_signal(S_Concentration_Broken, spell_id)

	return 0

def Sequence(char, args, evt_obj):
	d20a = evt_obj.get_d20_action()
	# TODO: check D20ADF_Breaks_Concentration
	return 0

def Recipient(char, args, evt_obj):
	return 0

def Stop(char, args, evt_obj):
	spell_id = args.get_arg(0)
	char.float_mesfile_line('mes\\combat.mes', 5060)
	char.d20_send_signal(S_Concentration_Broken, spell_id)
	return 0

def Remove(char, args, evt_obj):
	args.remove_spell()
	args.remove_spell_mod()
	return 0

def TT(char, args, evt_obj):
	evt_obj.append('Concentrating [Implosion]')
	return 0

def Radial(char, args, evt_obj):
	spell_id = args.get_arg(0)
	implode = tpdp.RadialMenuEntryPythonAction(
			'Implode', D20A_PYTHON_ACTION,
			implodeEnum, spell_id, 'TAG_INTERFACE_HELP')
	implode.add_child_to_standard(char, tpdp.RadialMenuStandardNode.Spells)
	return 0

def Perform(char, args, evt_obj):
	spell_id = args.get_arg(0)
	dc = args.get_arg(2)

	# concentrated this round
	args.set_arg(3, 1)

	target = evt_obj.d20a.target
	packet = tpdp.SpellPacket(spell_id)

	if packet.check_spell_resistance_force(target):
		game.particles('Fizzle', target)
		game.sound(17122,1)
		return 0

	caster = packet.caster

	saved = target.saving_throw_spell(
			dc, D20_Save_Fortitude, D20STD_F_NONE, caster, spell_id)

	if target.stat_level_get(stat_size) > STAT_SIZE_MEDIUM:
		game.particles('sp-Implosion-large', target)
	else:
		game.particles('sp-Implosion', target)
	game.sound(10847)

	if saved:
		game.sound(10849)
		return 0

	if target.type == obj_t_npc:
		target.obj_set_obj(obj_f_last_hit_by, caster)
	packet.remove_target(target)
	target.critter_kill()
	target.object_flag_set(OF_DESTROYED)

	return 0

def Frame(char, args, evt_obj):
	print "Frame"
	print dir(evt_obj)
	return 0

def EndTurn(char, args, evt_obj):
	if args.get_arg(3):
		args.set_arg(3, 0)
		return 0

	tb_status = tpactions.get_cur_seq().tb_status

	# Not enough time left to concentrate
	if tb_status.hourglass_state < 2:
		char.float_mesfile_line('mes\\combat.mes', 5060)
		#char.d20_send_signal(S_Concentration_Broken, spell_id)
		args.condition_remove()
		args.remove_spell_mod()

	return 0

# spell_id, duration, dc, concentrated, spare
cast = PythonModifier('sp-Implosion', 5, 0)
cast.AddHook(ET_OnConditionAddPre, EK_NONE, PreAdd, ())
cast.AddHook(ET_OnD20Query, EK_Q_Critter_Is_Concentrating, IsConcentrating, ())
cast.AddHook(ET_OnTakingDamage2, EK_NONE, DmgCheck, ())
cast.AddHook(ET_OnD20Signal, EK_S_Sequence, Sequence, ())
#cast.AddHook(ET_OnD20Signal, EK_S_Action_Recipient, Recipient, ())
cast.AddHook(ET_OnD20Signal, EK_S_Remove_Concentration, Stop, ())
cast.AddHook(ET_OnD20Signal, EK_S_Concentration_Broken, Remove, ())
cast.AddHook(ET_OnD20Signal, EK_S_Killed, Remove, ())
cast.AddHook(ET_OnD20Signal, EK_S_EndTurn, EndTurn, ())
cast.AddHook(ET_OnGetTooltip, EK_NONE, spellTooltip, ())
cast.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, Radial, ())
cast.AddHook(ET_OnD20PythonActionPerform, implodeEnum, Perform, ())
#cast.AddHook(ET_OnD20PythonActionFrame, implodeEnum, Frame, ())
cast.AddSpellCountdownStandardHook()

