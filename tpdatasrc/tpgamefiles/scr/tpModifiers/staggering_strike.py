from templeplus.pymod import PythonModifier
from __main__ import game
from toee import *
import tpdp

# Complete Adventurer: p. 112

def StaggeringStrikeFeatOnSneakAttack(attachee, args, evt_obj):
	args.set_arg(0, 1) #Set the flag to potentially apply the effect
	return 0 

def StaggeringStrikeFeatOnDamage(attachee, args, evt_obj):
	#Do nothing if sneak attack was not applied
	if not args.get_arg(0):
		return 0
	
	tgt = evt_obj.attack_packet.target
	if tgt == OBJ_HANDLE_NULL:
		return 0
	
	args.set_arg(0, 0)
	
	damage = evt_obj.damage_packet.final_damage
	
	game.create_history_freeform(tgt.description + " staggering strike...\n\n")
	
	#Fortitude save to avoid the effect
	if tgt.saving_throw( damage, D20_Save_Fortitude, D20STD_F_NONE, attachee):
		game.create_history_freeform("target resisted.\n\n")
	else:
		attachee.float_text_line("Staggering strike!")
		game.create_history_freeform("target Staggered!\n\n")
		tgt.condition_add("Staggering Strike Effect", 1)
		
	return 0 
	
def StaggeringStrikeBeginRound(attachee, args, evt_obj):
	# Reset the already used this round flag
	args.set_arg(0, 0)
	return 0

def StaggeringStrikeEffectBeginRound(attachee, args, evt_obj):
	duration = args.get_arg(0)
	
	#If zero rounds remaining remove the effect (to avoid it lasting forever outside of combat)
	if duration < 1:
		args.condition_remove()
	
	#Decrement the duration
	duration = duration - 1
	args.set_arg(0, duration)
	
	return 0

def StaggeringStrikeEffectTurnBasedStatusInit(attachee, args, evt_obj):
	#Remove the target's move action
	evt_obj.tb_status.hourglass_state = 2 # sets to Standard Action Only
	
	#Duration is decremented in On Begin Round Just check it here
	if duration < 1:
		args.condition_remove()
	
	return 0
	
def StaggeringStrikeEffectOnHealing(attachee, args, evt_obj):
	#Remove the effect if there is healing 
	args.condition_remove()
	return 0
	
def StaggeringStrikeEffectGetTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0

	# Set the tooltip
	evt_obj.append("Staggered")

	return 0

def StaggeringStrikeEffectGetEffectTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0

	# Set the tooltip
	evt_obj.append(tpdp.hash("STAGGERING_STRIKE"), -2, "")
	return 0

#Setup the feat
StaggeringStrikeFeat = PythonModifier("Staggering Strike Feat", 2) #Apply Effect, Extra
StaggeringStrikeFeat.MapToFeat("Staggering Strike")
StaggeringStrikeFeat.AddHook(ET_OnD20PythonSignal, "Sneak Attack Damage Applied", StaggeringStrikeFeatOnSneakAttack, ())
StaggeringStrikeFeat.AddHook(ET_OnDealingDamage2, EK_NONE, StaggeringStrikeFeatOnDamage, ())
StaggeringStrikeFeat.AddHook(ET_OnBeginRound, EK_NONE, StaggeringStrikeBeginRound, ())
StaggeringStrikeFeat.AddHook(ET_OnConditionAdd, EK_NONE, StaggeringStrikeBeginRound, ())

#Setup the effect
StaggeringStrikeEffect = PythonModifier("Staggering Strike Effect", 2) #Rounds, Extra
StaggeringStrikeEffect.AddHook(ET_OnTurnBasedStatusInit, EK_NONE, StaggeringStrikeEffectTurnBasedStatusInit, ())
StaggeringStrikeEffect.AddHook(ET_OnBeginRound, EK_NONE, StaggeringStrikeEffectBeginRound, ())
StaggeringStrikeEffect.AddHook(ET_OnReceiveHealing, EK_NONE, StaggeringStrikeEffectOnHealing, ())
StaggeringStrikeEffect.AddHook(ET_OnGetTooltip, EK_NONE, StaggeringStrikeEffectGetTooltip, ())
StaggeringStrikeEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, StaggeringStrikeEffectGetEffectTooltip, ())
