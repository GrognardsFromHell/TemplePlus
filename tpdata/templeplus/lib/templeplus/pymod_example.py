from templeplus.pymod import PythonModifier
from toee import *
import tpdp

def OnInit(attachee, args, evtObj):
	print "I've been inited! My attachee: " + str(attachee) 
	#print str(args)
	return 0

def RadialMenuEntry(attachee, args, evtObj):
	#print "My radial menu is being built! Attachee: " + str(attachee)
	radialParent = tpdp.RadialMenuEntryParent(143) # combat.mes line
	radialParentId = radialParent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
	radialAction = tpdp.RadialMenuEntryAction(5055, D20A_CAST_SPELL, 0, "TAG_INTERFACE_HELP")
	radialAction.add_as_child(attachee, radialParentId)
	return 0


def BeginRound(attachee, args, evtObj):
	#print "Callback for Character's start of round! Note: this gets called in realtime too, every 6 seconds. Useful for Spell tickdowns."
	#print "Num ticked: " + str(evtObj.data1) # evtObj.data1 will contain the number of rounds "ticked"
	return 0


def EffectTooltip(attachee, args, evtObj):
	evtObj.append(54, 1, "\nExtra Effect Text")
	# first arg: up to 90 are buffs (above portrait), then up to 167 are debuffs (below portrait), above are effects "inside" the portrait
	# second arg: spell enum
	return 0

pmEx = PythonModifier("PyMod Example", 0)
pmEx.AddHook(ET_OnConditionAdd, EK_NONE, OnInit, (1,))
pmEx.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, RadialMenuEntry, ())
pmEx.AddHook(ET_OnBeginRound, EK_NONE, BeginRound, ())
pmEx.AddHook(ET_OnGetEffectTooltip, EK_NONE, EffectTooltip, ())