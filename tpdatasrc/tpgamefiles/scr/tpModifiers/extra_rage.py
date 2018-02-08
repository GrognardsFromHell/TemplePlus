#Extra Rage:  Complete Warrior, p. 98

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Extra Rage"

def ExtraRageNewDay(attachee, args, evt_obj):
	ExtraRageCount = attachee.has_feat("Extra Rage")
	
	#Extra Rage grands 2 additional uses or rage each time the feat is taken
	args.set_arg(0, args.get_arg(0) + 2 * ExtraRageCount)
	return 0

extendRageFeat = PythonModifier()
extendRageFeat.ExtendExisting("Barbarian_Rage")
extendRageFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, ExtraRageNewDay, ())
