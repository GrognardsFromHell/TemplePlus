from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Divine Vigor:  Complete Warrior, p. 108

divineVigorEnum = 2601

print "Registering Divine Vigor"

def DivineVigorRadial(attachee, args, evt_obj):
	isAdded = attachee.condition_add_with_args("Divine Vigor Effect",0,0) # adds the "Divine Vigor" condition on first radial menu build
	radialAction = tpdp.RadialMenuEntryPythonAction(-1, D20A_PYTHON_ACTION, divineVigorEnum, 0, "TAG_INTERFACE_HELP")
	radialAction.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def OnDivineVigorCheck(attachee, args, evt_obj):
	#Get the current number of turn charges
	TurnCharges = attachee.d20_query("Turn Undead Charges")

	#Check for remaining turn undead attempts
	if (TurnCharges < 1):
		evt_obj.return_val = AEC_OUT_OF_CHARGES
		return 0

	#Check that the character is not a fallen paladin without black guard levels
	if attachee.d20_query(Q_IsFallenPaladin) and (attachee.stat_level_get(stat_level_blackguard) == 0):
		evt_obj.return_val = AEC_INVALID_ACTION
		return 0

	return 1

def OnDivineVigorPerform(attachee, args, evt_obj):
	#Deduct a turn undead charge
	attachee.d20_send_signal("Deduct Turn Undead Charge")
	
	#Set to active (arg 1)
	args.set_arg(0, 1) 	

	#Duration (arg 2) = Charisma bonus minutes (1 minute = 10 rounds)
	numRounds = 10 * attachee.stat_level_get(stat_cha_mod)
	args.set_arg(1, numRounds)
	
	#Temporary Hit Points (arg 3) = 2 * character level
	tempHP = attachee.stat_level_get(stat_level) * 2
	args.set_arg(2, tempHP)
	
	return 0

	
	
def DivineVigorBeginRound(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0): 
		return 0

	#calculate the number of rounds left
	numRounds = args.get_arg(1)
	roundsToReduce = evt_obj.data1
	if numRounds - roundsToReduce > 0:
		args.set_arg(1, numRounds - roundsToReduce ) #decrement the number of rounds
		return 0
	else:
		args.set_arg(0, 0) # set inactive
		args.set_arg(1, 0) # set to zero rounds
		args.set_arg(2, 0) # set to zero temporary hit points
	return 0


def DivineVigorTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0
		
	tempHP = args.get_arg(2)

	# Set the tooltip (showing temporary hit points if applicable)
	if tempHP > 0:
		evt_obj.append("Divine Vigor Temp HP: " + str(tempHP))
	else:
		evt_obj.append("Divine Vigor")
	return 0

def DivineVigorEffectTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0
		
	tempHP = args.get_arg(2)

	# Set the tooltip (showing temporary hit points if applicable)
	if tempHP > 0:
		evt_obj.append(tpdp.hash("DIVINE_VIGOR"), -2, " (" + str(args.get_arg(1)) + " rounds, Temp HP:" + str(tempHP) + ")")
	else:
		evt_obj.append(tpdp.hash("DIVINE_VIGOR"), -2, " (" + str(args.get_arg(1)) + " rounds)")
	return 0
	
def DivineVigorMoveSpeed(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0
		
	#Movement speed increased by 10 feet (enhancement bonus)
	evt_obj.bonus_list.add(10, 12, 137)
	return 0
	
def DivineVigorTakingDamage2(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0
	
	tempHP = args.get_arg(2)
	
	if tempHP <= 0:
		return 0
	finalDam = evt_obj.damage_packet.final_damage
	hpLeft = tempHP - finalDam
	
	#Zero out if hitpoins are less than zero
	if hpLeft < 0:
		hpLeft = 0
	args.set_arg(2, hpLeft)
	
	if hpLeft <= 0:
		evt_obj.damage_packet.final_damage -= tempHP
		evt_obj.damage_packet.add_damage_bonus(-tempHP, 0, 154)
		attachee.float_text_line("Damage Absorbed " + str(tempHP) + ".")
		attachee.d20_send_signal(S_Temporary_Hit_Points_Removed, args.get_arg(0))
		return 0
	
	attachee.float_text_line("Damage Absorbed " + str(finalDam) + ".")
	evt_obj.damage_packet.final_damage = 0
	evt_obj.damage_packet.add_damage_bonus(-finalDam, 0, 154)
	
	return 0
	
def DivineVigorHasTemporaryHitpoints(attachee, args, evt_obj):
	evt_obj.return_val = 0 #Default to false
	
	#Set to true if divine vigor is active and hit points remain
	if args.get_arg(0) > 0 and args.get_arg(2) > 0:
		evt_obj.return_val = 1
		return 0
	return 0

#Setup the feat
divineVigorFeat = PythonModifier("Divine Vigor Feat", 2) 
divineVigorFeat.MapToFeat("Divine Vigor")
divineVigorFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, DivineVigorRadial, ())

#Setup the effect
divineVigorEffect = PythonModifier("Divine Vigor Effect", 3)
divineVigorEffect.AddHook(ET_OnD20PythonActionCheck, divineVigorEnum, OnDivineVigorCheck, ())
divineVigorEffect.AddHook(ET_OnD20PythonActionPerform, divineVigorEnum, OnDivineVigorPerform, ())
divineVigorEffect.AddHook(ET_OnBeginRound, EK_NONE, DivineVigorBeginRound, ())
divineVigorEffect.AddHook(ET_OnGetTooltip, EK_NONE, DivineVigorTooltip, ())
divineVigorEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, DivineVigorEffectTooltip, ())
divineVigorEffect.AddHook(ET_OnGetMoveSpeed, EK_NONE, DivineVigorMoveSpeed, ())
divineVigorEffect.AddHook(ET_OnTakingDamage2, EK_NONE, DivineVigorTakingDamage2, ())
divineVigorEffect.AddHook(ET_OnD20Query, EK_Q_Has_Temporary_Hit_Points, DivineVigorHasTemporaryHitpoints, ())
