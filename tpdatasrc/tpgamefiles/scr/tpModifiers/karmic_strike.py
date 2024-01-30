from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Karmic Strike:  Complete Warrior, p. 102

print "Registering Karmic Strike"

def KarmicStrikeInitialize(attachee, args, evt_obj):
	args.set_arg(0, 0)
	args.set_arg(1, 0)
	return 0

def KarmicStrikeRadial(attachee, args, evt_obj):
	#Add a checkbox to turn on and off karmic strike
	checkboxKarmicStrike = tpdp.RadialMenuEntryToggle("Karmic Strike", "TAG_INTERFACE_HELP")
	checkboxKarmicStrike.link_to_args(args, 0)
	checkboxKarmicStrike.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def KarmicStrikeTakingDamage(attachee, args, evt_obj):
	if args.get_arg(0) <= 0:
		return 0
	
	action_type = evt_obj.attack_packet.action_type
	
	if action_type == D20A_UNSPECIFIED_ATTACK or action_type == D20A_FULL_ATTACK or action_type == D20A_STANDARD_ATTACK or action_type == D20A_TOUCH_ATTACK:
		opponent = evt_obj.attack_packet.attacker
		args.set_arg(1, 1) #Set allow multiple AOOs
		attachee.make_aoo_if_possible(opponent)
		args.set_arg(1, 0) #Clears multiple AOOs
	return 0
	
def KarmicStrikeACPenalty(attachee, args, evt_obj):
	if args.get_arg(0):
		evt_obj.bonus_list.add(-4 , 0, "Karmic Strike") #Unnamed penalty
	return 0

def AllowMultipleAOOs(attachee, args, evt_obj):
	if args.get_arg(1):
		evt_obj.return_val = 1
	else:
		evt_obj.return_val = 0
	return 0

	
#Setup the feat
KarmicStrikeFeat = PythonModifier("Karmic Strike Feat", 2) # active flag, doing AOO
KarmicStrikeFeat.MapToFeat("Karmic Strike")
KarmicStrikeFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, KarmicStrikeRadial, ())
KarmicStrikeFeat.AddHook(ET_OnTakingDamage2, EK_NONE, KarmicStrikeTakingDamage, ())
KarmicStrikeFeat.AddHook(ET_OnGetAC, EK_NONE, KarmicStrikeACPenalty, ())
KarmicStrikeFeat.AddHook(ET_OnConditionAdd, EK_NONE, KarmicStrikeInitialize, ())
KarmicStrikeFeat.AddHook(ET_OnD20PythonQuery, "Allow Multiple AOOs", AllowMultipleAOOs, ())