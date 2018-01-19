from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering PyMod Example"

''' Crash course!
 ToEE uses an Event system to handle modifiers. 

 Example: When ToEE calculates a critter's AC, it runs a GetAC event.
  To modify the result, we'll use an OnGetAC event handler.
 
  The event handler takes an Event Object as input, 
  and modifies it to add bonuses to AC.
 
  This event object gets passed around to all the various event handlers,
  each in turn adding its own pluses (which can be negative also).
 
  The internal C function will then sum up all the bonuses and get the value.
 
 Each event type generally has its own type of Event Object - 
  each with different properties and methods.
  Consult the Temple+ Wiki for full info.
  
 Or , just learn by example from here :)
'''

def OnInit(attachee, args, evt_obj):
	print "I've been inited! My attachee: " + str(attachee) 
	#print str(args)
	return 0

def RadialMenuEntry(attachee, args, evt_obj):
	#print "My radial menu is being built! Attachee: " + str(attachee)

	# add a parent node labeled "Attempt Succeeds!" (combat.mes line 143) to the Class standard node
	radialParent = tpdp.RadialMenuEntryParent(143) # combat.mes line
	radialParentId = radialParent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
	
	#add a child action node to the above node, labeled "Guidance" (combat.mes line 5055), that triggers a Cast Spell action
	radialAction = tpdp.RadialMenuEntryAction(5055, D20A_CAST_SPELL, 0, "TAG_INTERFACE_HELP")
	radialAction.add_as_child(attachee, radialParentId)
	return 0


def BeginRound(attachee, args, evt_obj):
	#print "Callback for Character's start of round! Note: this gets called in realtime too, every 6 seconds. Useful for Spell tickdowns."
	#print "Num ticked: " + str(evt_obj.data1) # evt_obj.data1 will contain the number of rounds "ticked"
	return 0


def EffectTooltip(attachee, args, evt_obj):
	spellEnum = 2
	evt_obj.append(54, spellEnum , "\nExtra Effect Text") # use this for spells
	evt_obj.append(54, -2, "Effect Text") # use -2 in the second arg for non-spell effects
	# first arg: indicator type
	#      range up to 90 is for buffs (indicators above portrait)
	#      then up to 167 are debuffs (below portrait)
	#      above are effects "inside" the portrait
	# second arg: spell enum
	return 0

# TurnBasedStatusInit is used for initializing the "hourglass" state (Full Round Action, Partial Charge, Standard Action, Move Action, 5' step only) and "surplus" move distance (from when you used a move action and have remaining move), as well as other flags (TBSF_ flags in constants.py)
def TurnBasedStatusInit(attachee, args, evt_obj):
	evt_obj.tb_status.hourglass_state = 2 # sets to Standard Action Only
	evt_obj.tb_status.surplus_move_dist = 16.0
	
def GetArmorClass(attachee, args, evt_obj):
	evt_obj.bonus_list.add(-10, 0, 180) # value, bonus type, bonus.mes line
	

pmEx = PythonModifier("PyMod Example", 0) # creates and registers a condition with 0 args; prevents duplicates!
pmEx.AddHook(ET_OnConditionAdd, EK_NONE, OnInit, (1,))
pmEx.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, RadialMenuEntry, ())
pmEx.AddHook(ET_OnBeginRound, EK_NONE, BeginRound, ())
pmEx.AddHook(ET_OnGetEffectTooltip, EK_NONE, EffectTooltip, ())
pmEx.AddHook(ET_OnTurnBasedStatusInit, EK_NONE, TurnBasedStatusInit, ())
pmEx.AddHook(ET_OnGetAC, EK_NONE, GetArmorClass, ())

