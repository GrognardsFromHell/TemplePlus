from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Complete Warrior p.106

divineShieldEnum = 2602

print "Registering Divine Shield"

#Check that the the PC has a shield
def HasShield(pc):
	if pc.item_worn_at(item_wear_shield) == OBJ_HANDLE_NULL:
		return 0
	return 1

def DivineShieldRadial(attachee, args, evt_obj):
	isAdded = attachee.condition_add_with_args("Divine Shield Effect",0,0) # adds the "Divine Shield" condition on first radial menu build
	radialAction = tpdp.RadialMenuEntryPythonAction(-1, D20A_PYTHON_ACTION, divineShieldEnum, 0, "TAG_INTERFACE_HELP")
	radialAction.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0
	
def OnDivineShieldCheck(attachee, args, evt_obj):
	#First, a shield must be wielded
	if not HasShield(attachee):
		evt_obj.return_val = AEC_INVALID_ACTION
		return 0
	
	TurnCharges = attachee.d20_query("Turn Undead Charges")
	
	#Second, check for remaining turn undead attempts
	if (TurnCharges < 1):
		evt_obj.return_val = AEC_OUT_OF_CHARGES
		return 0
	
	#Third, check that the character is not a fallen paladin without black guard levels
	if attachee.d20_query(Q_IsFallenPaladin) and (attachee.stat_level_get(stat_level_blackguard) == 0):
		evt_obj.return_val = AEC_INVALID_ACTION
		return 0
	
	return 1
	
def OnDivineShieldPerform(attachee, args, evt_obj):
	#Set to active
	args.set_arg(0, 1) 
	
	#Deduct a turn undead charge
	attachee.d20_send_signal("Deduct Turn Undead Charge")
	
	#Duration is half the character level in rounds
	numRounds = attachee.stat_level_get(stat_level) / 2
	if numRounds < 1:
		numRounds = 1
	args.set_arg(1, numRounds)
	
	return 0
	
def DivineShieldAcBonus(attachee, args, evt_obj):
	
	# not active, do nothing
	if not args.get_arg(0): 
		return 0
	
	item = attachee.item_worn_at(item_wear_shield)
	
	#No shield, no bonus
	if  item == OBJ_HANDLE_NULL:
		return 0
	
	#Is the shield a buckler?
	hasBuckler = item.is_buckler()
	
	#If the buckler bonus is disabled, no bonus
	if hasBuckler:
		bucklerDisabled = attachee.d20_query("Buckler Bonus Disabled")
		if bucklerDisabled == 1:
			return 0
	
	#Add the charisma modifier to the shield's bonus
	charisma = attachee.stat_level_get(stat_charisma)
	
	charismaBonus = (charisma - 10) / 2
	
	# This doesn't work exactly like the feat.  Instead of adding the bonus to the shield's AC bonus,
	# the bonus is added seperately as a bonus that always stacks.  This should work out the same in almost
	# all cases.
	if charismaBonus > 0:
		evt_obj.bonus_list.add(charismaBonus, 0, "Divine Shield Feat")

	return 0
	
def DivineShieldInventoryUpdate(attachee, args, evt_obj):
	if not HasShield(attachee):
		args.set_arg(0, 0) # set inactive
		args.set_arg(1, 0) # set to zero rounds
		return 0
	return 0
	
def DivineShieldBeginRound(attachee, args, evt_obj):
	
	# not active, do nothing
	if not args.get_arg(0): 
		return 0

	#calculate the number of rounds left
	numRounds = args.get_arg(1)
	roundsToReduce = evt_obj.data1
	
	if (numRounds - roundsToReduce) > 0:
		args.set_arg(1, numRounds - roundsToReduce ) #decrement the number of rounds
	else:
		args.set_arg(0, 0) # set inactive
		args.set_arg(1, 0) # set to zero rounds
	return 0
	
def DivineShieldTooltip(attachee, args, evt_obj):

	# not active, do nothing
	if not args.get_arg(0):
		return 0
	
	# Set the tooltip
	evt_obj.append("Divine Shield (" + str(args.get_arg(1)) + " rounds)")

	return 0
	
def DivineShieldEffectTooltip(attachee, args, evt_obj):
	
	# not active, do nothing
	if not args.get_arg(0):
		return 0
	
	# Set the tooltip
	evt_obj.append(tpdp.hash("DIVINE_SHIELD"), -2, " (" + str(args.get_arg(1)) + " rounds)")
	return 0

#Setup the feat
divineShieldFeat = PythonModifier("Divine Shield Feat", 2) 
divineShieldFeat.MapToFeat("Divine Shield")
divineShieldFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, DivineShieldRadial, ())

#Setup the condition added by the feat
divineShieldEffect = PythonModifier("Divine Shield Effect", 2)
divineShieldEffect.AddHook(ET_OnD20PythonActionCheck, divineShieldEnum, OnDivineShieldCheck, ())
divineShieldEffect.AddHook(ET_OnD20PythonActionPerform, divineShieldEnum, OnDivineShieldPerform, ())
divineShieldEffect.AddHook(ET_OnGetAC, EK_NONE, DivineShieldAcBonus, ())
divineShieldEffect.AddHook(ET_OnBeginRound, EK_NONE, DivineShieldBeginRound, ())
divineShieldEffect.AddHook(ET_OnGetTooltip, EK_NONE, DivineShieldTooltip, ())
divineShieldEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, DivineShieldEffectTooltip, ())
divineShieldEffect.AddHook(ET_OnD20Signal, EK_S_Inventory_Update, DivineShieldInventoryUpdate, ())

