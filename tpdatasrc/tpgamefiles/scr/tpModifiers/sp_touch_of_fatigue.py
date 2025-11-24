from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
print "Registering sp_touch_of_fatigue"

# args: (0-4)
# 0 - spell_id
# 1 - duration
# 2 - touch attack after case
# 3 - spare

def TouchOfFatigueTooltip(attachee, args, evt_obj):
	# Set the tooltip
	evt_obj.append("Touch of Fatigue")
	return 0
	
def TouchOfFatigueEffectTooltip(attachee, args, evt_obj):
	# Set the tooltip
	evt_obj.append(tpdp.hash("TOUCH_OF_FATIGUE"), -2, "")
	return 0
	
def TouchOfFatigueHasSpellActive(attachee, args, evt_obj):
	if evt_obj.data1 == 1001:
		evt_obj.return_val = 1
	return 0
	
def TouchOfFatigueHoldingCharge(attachee, args, evt_obj):
	spell_id = args.get_arg(0)

	evt_obj.return_val = 1
	evt_obj.data1 = spell_id
	return 0
	
def TouchOfFatigueTouchAttackAdded(attachee, args, evt_obj):
	if evt_obj.data1 != 1001:
		args.condition_remove()
		args.remove_spell()
	return 0
	
def TouchOfFatigueTouchAttack(attachee, args, evt_obj):
	addFatigue = 0
	
	#Touch of fatigue does not allow upgrading
	action = evt_obj.get_d20_action()
	target = action.target
	wrongType = target.is_category_type(mc_type_undead) or target.is_category_type(mc_type_construct) or target.is_category_type(mc_type_ooze) or target.is_category_type(mc_type_plant)
	hasFatigueExhaust = target.d20_query("Fatigued") or target.d20_query("Exhausted")
	
	if wrongType:
		target.float_text_line("Fatigue Immunity")
	
	if hasFatigueExhaust:
		target.float_text_line("Already Fatigued")
	
	if not wrongType and not hasFatigueExhaust:
		spell_id = args.get_arg(0)
		packet = tpdp.SpellPacket(spell_id)
		if not target.saving_throw_spell( packet.dc, D20_Save_Fortitude, D20STD_F_NONE, attachee, packet.spell_id ):
			duration = args.get_arg(1) 
			target.condition_add_with_args("FatigueExhaust", 0, duration, 0, 1, 0, 0)
		else:
			target.float_mesfile_line( 'mes\\spell.mes', 30001 )
			game.particles( 'Fizzle', target )
			
	args.condition_remove()
	args.remove_spell_with_key(EK_S_Concentration_Broken)
	return 0
	
def TouchOfFatigueSpellCast(attachee, args, evt_obj):
	#End the spell if the character casts another spell
	spell_id = evt_obj.data1
	packet = tpdp.SpellPacket(spell_id)
	if packet.caster == attachee:
		args.condition_remove()
		args.remove_spell()
	return 0
	
def TouchOfFatigueConditionAdd(attachee, args, evt_obj):
	tbFlags = tpdp.cur_seq_get_turn_based_status_flags()
	tbFlags = tbFlags | TBSF_TouchAttack
	tpdp.cur_seq_set_turn_based_status_flags(tbFlags)
	attachee.d20_send_signal(S_TouchAttackAdded, 1001)
	return 0

TouchOfFatigue = PythonModifier("sp-Touch of Fatigue", 4)
TouchOfFatigue.AddHook(ET_OnGetTooltip, EK_NONE, TouchOfFatigueTooltip, ())
TouchOfFatigue.AddHook(ET_OnGetEffectTooltip, EK_NONE, TouchOfFatigueEffectTooltip, ())
TouchOfFatigue.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, TouchOfFatigueHasSpellActive, ())
TouchOfFatigue.AddHook(ET_OnD20Query, EK_Q_HoldingCharge, TouchOfFatigueHoldingCharge, ())
TouchOfFatigue.AddHook(ET_OnD20Signal, EK_S_TouchAttackAdded, TouchOfFatigueTouchAttackAdded, ())
TouchOfFatigue.AddHook(ET_OnD20Signal, EK_S_TouchAttack, TouchOfFatigueTouchAttack, ())
TouchOfFatigue.AddHook(ET_OnD20Signal, EK_S_Spell_Cast, TouchOfFatigueSpellCast, ())
TouchOfFatigue.AddHook(ET_OnConditionAdd, EK_NONE, TouchOfFatigueConditionAdd, ())
TouchOfFatigue.AddSpellDispellCheckHook()
TouchOfFatigue.AddSpellTouchAttackDischargeRadialMenuHook()

