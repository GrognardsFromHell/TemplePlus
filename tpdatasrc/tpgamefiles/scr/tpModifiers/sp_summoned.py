
from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering sp-Summoned extension"

def Banish(critter, args, evt_obj):
	critter.critter_banish()
	return 0

summoned = PythonModifier() 
summoned.ExtendExisting('sp-Summoned')
summoned.AddHook(ET_OnConditionRemove, EK_NONE, Banish, ())
summoned.AddHook(ET_OnD20Signal, EK_S_Killed, Banish, ())
