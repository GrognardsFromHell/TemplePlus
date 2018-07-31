from templeplus.pymod import PythonModifier
from __main__ import game
from toee import *
import tpdp

# Complete Warrior: p. 103

def PainTouchFeatOnSpecialAttack(attachee, args, evt_obj):

	#Check that the attack is a stunning fist
	if evt_obj.attack != 1:
		return 0

	tgt = evt_obj.target
	
	sizeTarget = tgt.get_size
	sizeAttacker = attachee.get_size
	
	#Target must be no more than one size category greater than the attacker
	if sizeAttacker + 1 >= sizeTarget:
		game.create_history_freeform(tgt.description + " effected by pain touch!\n\n")
		#Note:  The condition lasts for 2 rounds but the first round that victim is stunned so it in effect lasts 1 round
		tgt.condition_add("Pain Touch Effect", 2)
	else:
		game.create_history_freeform(tgt.description + " too large, uneffected by pain touch!\n\n")
	
	return 0 

def PainTouchEffectBeginRound(attachee, args, evt_obj):
	duration = args.get_arg(0)
	
	#If zero rounds remaining remove the effect (to avoid it lasting forever outside of combat)
	if duration < 1:
		args.condition_remove()
	
	#Decrement the duration
	roundsToReduce = evt_obj.data1
	
	duration = duration - roundsToReduce
	duration = max(duration, 0)
	args.set_arg(0, duration)
	
	return 0

def PainTouchEffectTurnBasedStatusInit(attachee, args, evt_obj):
	#Remove the target's standard action
	if evt_obj.tb_status.hourglass_state > 1:
		evt_obj.tb_status.hourglass_state = 1 # sets to Move Action Only
	
	
	#Duration is decremented in On Begin Round Just check it here
	duration = args.get_arg(0)
	if duration < 1:
		args.condition_remove()
	
	return 0
	
def PainTouchEffectGetTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0

	# Set the tooltip
	evt_obj.append("Nauseated!")

	return 0

def PainTouchEffectGetEffectTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0

	# Set the tooltip
	evt_obj.append(tpdp.hash("PAIN_TOUCH"), -2, "")
	return 0

#Setup the feat
PainTouchFeat = PythonModifier("Pain Touch Feat", 2) #Extra, Extra
PainTouchFeat.MapToFeat("Pain Touch")
PainTouchFeat.AddHook(ET_OnSpecialAttack, EK_NONE, PainTouchFeatOnSpecialAttack, ())

#Setup the effect
PainTouchEffect = PythonModifier("Pain Touch Effect", 2) #Rounds, Extra
PainTouchEffect.AddHook(ET_OnTurnBasedStatusInit, EK_NONE, PainTouchEffectTurnBasedStatusInit, ())
PainTouchEffect.AddHook(ET_OnBeginRound, EK_NONE, PainTouchEffectBeginRound, ())
PainTouchEffect.AddHook(ET_OnGetTooltip, EK_NONE, PainTouchEffectGetTooltip, ())
PainTouchEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, PainTouchEffectGetEffectTooltip, ())
