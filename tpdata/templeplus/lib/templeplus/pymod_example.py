from templeplus.pymod import PythonModifier
from toee import *

def OnInit(attachee, args):
	print "I've been inited! My attachee: " + str(attachee) 
	#print str(args)


def RadialMenuEntry(attachee, args):
	#print "My radial menu is being built! Attachee: " + str(attachee)
	#print str(args)
	pass


def BeginRound(attachee, args, evtObj):
	#print "Callback for Character's start of round! Note: this gets called in realtime too, every 6 seconds. Useful for Spell tickdowns."
	print "Num ticked: " + str(evtObj.data1) # evtObj.data1 will contain the number of rounds "ticked"


def EffectTooltip(attachee, args, evtObj):
	evtObj.append(54, -1, "\nExtra Effect Text")
	# first arg: up to 90 are buffs (above portrait), then up to 167 are debuffs (below portrait), above are effects "inside" the portrait
	# second arg: spell enum

pmEx = PythonModifier("PyMod Example", 0)
pmEx.AddHook(ET_OnConditionAdd, EK_NONE, OnInit, (1,))
pmEx.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, RadialMenuEntry, ())
pmEx.AddHook(ET_OnBeginRound, EK_NONE, BeginRound, ())
pmEx.AddHook(ET_OnGetEffectTooltip, EK_NONE, EffectTooltip, ())