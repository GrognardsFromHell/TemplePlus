from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Divine Armor:  Player's Handbook II, p. 88

divineArmorEnum = 2600

print "Registering Divine Armor"

def DivineArmorRadial(attachee, args, evt_obj):
	isAdded = attachee.condition_add_with_args("Divine Armor Effect",0,0) # adds the "Divine Armor" condition on first radial menu build
	radialAction = tpdp.RadialMenuEntryPythonAction(-1, D20A_PYTHON_ACTION, divineArmorEnum, 0, "TAG_INTERFACE_HELP")
	radialAction.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def OnDivineArmorCheck(attachee, args, evt_obj):
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

def OnDivineArmorPerform(attachee, args, evt_obj):
	#Set to active
	args.set_arg(0, 1) 

	#Deduct a turn undead charge
	attachee.d20_send_signal("Deduct Turn Undead Charge")

	#Lasts one round always
	args.set_arg(1, 1)

	return 0

def DivineArmorBeginRound(attachee, args, evt_obj):
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
	return 0

def DivineArmorDamageReduction(attachee, args, evt_obj):
	if not args.get_arg(0): 
		return 0

	dr = 5  #Divine Armor Provides DR 5
	evt_obj.damage_packet.add_physical_damage_res(dr, 1, 126 ) # type 1 - will always apply
	return 0

def DivineArmorTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0

	# Set the tooltip
	evt_obj.append("Divine Armor")

	return 0

def DivineArmorEffectTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0

	# Set the tooltip
	evt_obj.append(tpdp.hash("DIVINE_ARMOR"), -2, "")
	return 0

#Hookup Setup the feat
divineArmorFeat = PythonModifier("Divine Armor Feat", 2) 
divineArmorFeat.MapToFeat("Divine Armor")
divineArmorFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, DivineArmorRadial, ())

#Setup the radial button
divineArmorEffect = PythonModifier("Divine Armor Effect", 2)
divineArmorEffect.AddHook(ET_OnD20PythonActionCheck, divineArmorEnum, OnDivineArmorCheck, ())
divineArmorEffect.AddHook(ET_OnD20PythonActionPerform, divineArmorEnum, OnDivineArmorPerform, ())
divineArmorEffect.AddHook(ET_OnBeginRound, EK_NONE, DivineArmorBeginRound, ())
divineArmorEffect.AddHook(ET_OnGetTooltip, EK_NONE, DivineArmorTooltip, ())
divineArmorEffect.AddHook(ET_OnTakingDamage2, EK_NONE, DivineArmorDamageReduction, ())
divineArmorEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, DivineArmorEffectTooltip, ())