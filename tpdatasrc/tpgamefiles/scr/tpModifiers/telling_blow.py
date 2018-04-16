#Telling Blow:  Player's Handbook II, p. 83
#Makes critical hits an additional condition for getting sneak attack damage.

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Telling Blow"

#Used by temple+.  Returns 1 if sneak attack damage should be done on a critial.
#Always returns true when the character has the telling blow feat.
def SneakAttackOnCritical(attachee, args, evt_obj):
	evt_obj.return_val = 1 #Turn On For Telling Blow
	return 0

tellingBlow = PythonModifier("Telling Blow", 2) # args are just-in-case placeholders
tellingBlow.MapToFeat("Telling Blow")
tellingBlow.AddHook(ET_OnD20PythonQuery, "Sneak Attack Critical", SneakAttackOnCritical, ())
