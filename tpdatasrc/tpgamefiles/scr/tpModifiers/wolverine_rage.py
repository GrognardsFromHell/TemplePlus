from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Wolverine's Rage:  Complete Divine, p. 86

WolverineRageEnum = 2700

print "Registering Wolverine's Rage"

def WolverineRageRadial(attachee, args, evt_obj):
	isAdded = attachee.condition_add_with_args("Wolverine Rage Effect",0,0) # adds the "Wolverine Rage" condition on first radial menu build
	radialAction = tpdp.RadialMenuEntryPythonAction(-1, D20A_PYTHON_ACTION, WolverineRageEnum, 0, "TAG_INTERFACE_HELP")
	radialAction.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0
	
def WolverineRageEffectTakingDamage2(attachee, args, evt_obj):
	finalDamage = evt_obj.damage_packet.final_damage
	
	#Set damage taken flag for next turn
	if finalDamage > 0:
		#AOO handling - set enabled flag directly
		if evt_obj.attack_packet.get_flags() & D20CAF_ATTACK_OF_OPPORTUNITY:
			args.set_arg(1, 1)
		else:
			args.set_arg(0, 1)
	return 0

def OnWolverineRageEffectCheck(attachee, args, evt_obj):
	#Don't allow if the effect is already enabled
	if args.get_arg(2):
		evt_obj.return_val = AEC_INVALID_ACTION
		return 0
	
	#Check that the character has taken damage this round
	if not args.get_arg(1):
		evt_obj.return_val = AEC_INVALID_ACTION
		return 0
	
	#Get the current number of wild shape charges
	WildShapeCharges = attachee.d20_query("Wild Shape Charges")

	#Check for remaining wild shape attempts
	if (WildShapeCharges < 1):
		evt_obj.return_val = AEC_OUT_OF_CHARGES
		return 0

	return 1

def OnWolverineRageEffectPerform(attachee, args, evt_obj):
	#Deduct a wild shape charge
	attachee.d20_send_signal("Deduct Wild Shape Charge")
	
	#Set to active
	args.set_arg(2, 1)

	#Lasts 5 rounds
	args.set_arg(3, 5)
	
	return 0

def WolverineRageEffectBeginRound(attachee, args, evt_obj):
	#Update the enabled flag for this round and clear the damage counter
	damageThisTurnFlag = args.get_arg(0)
	args.set_arg(1, damageThisTurnFlag)
	args.set_arg(0, 0)
	
	# not active, nothing more to do
	if not args.get_arg(2): 
		return 0

	#calculate the number of rounds left
	numRounds = args.get_arg(3)
	roundsToReduce = evt_obj.data1
	if numRounds - roundsToReduce > 0:
		args.set_arg(3, numRounds - roundsToReduce ) #decrement the number of rounds
	else:
		args.set_arg(2, 0) # set effect to inactive
	return 0

def WolverineRageEffectTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(2):
		return 0
		
	evt_obj.append("Wolverine's Rage")

	return 0

def WolverineRageEffectTooltipEffect(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(2):
		return 0

	# Set the tooltip
	evt_obj.append(tpdp.hash("WOLVERINE_RAGE"), -2, "")
	return 0
	
def WolverineRageEffectConMod(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(2):
		return 0

	evt_obj.bonus_list.add(2, 0, "Wolverine's Rage") # Unnamed bonus
	return 0
	
def WolverineRageEffectStrMod(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(2):
		return 0

	evt_obj.bonus_list.add(2, 0, "Wolverine's Rage")  # Unnamed bonus
	return 0
	
def WolverineRageEffectACPenalty(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(2):
		return 0

	evt_obj.bonus_list.add(-2, 0, "Wolverine's Rage")
	return 0

#Setup the feat
WolverineRageFeat = PythonModifier("Wolverine Rage Feat", 2) # spare, spare
WolverineRageFeat.MapToFeat("Wolverine's Rage")
WolverineRageFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, WolverineRageRadial, ())

#Setup the effect
WolverineRageEffect = PythonModifier("Wolverine Rage Effect", 5) #damage taken this round flag, feat enabled flag, effect active, rounds left, spare
WolverineRageEffect.AddHook(ET_OnTakingDamage2, EK_NONE, WolverineRageEffectTakingDamage2, ())
WolverineRageEffect.AddHook(ET_OnBeginRound, EK_NONE, WolverineRageEffectBeginRound, ())
WolverineRageEffect.AddHook(ET_OnD20PythonActionCheck, WolverineRageEnum, OnWolverineRageEffectCheck, ())
WolverineRageEffect.AddHook(ET_OnD20PythonActionPerform, WolverineRageEnum, OnWolverineRageEffectPerform, ())
WolverineRageEffect.AddHook(ET_OnGetTooltip, EK_NONE, WolverineRageEffectTooltip, ())
WolverineRageEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, WolverineRageEffectTooltipEffect, ())
WolverineRageEffect.AddHook(ET_OnAbilityScoreLevel, EK_STAT_CONSTITUTION, WolverineRageEffectConMod, ())
WolverineRageEffect.AddHook(ET_OnAbilityScoreLevel, EK_STAT_STRENGTH, WolverineRageEffectStrMod, ())
WolverineRageEffect.AddHook(ET_OnGetAC, EK_NONE, WolverineRageEffectACPenalty, ())