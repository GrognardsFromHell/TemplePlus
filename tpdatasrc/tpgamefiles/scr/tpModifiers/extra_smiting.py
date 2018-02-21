#Extra Smiting:  Complete Warrior, p. 98
#Note:  This version will give 2 smites to each smite ability.  Arguably 
#the wording could be interpreted differently.

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Extra Smiting"

def ExtraSmitingNewDayDestructionDomain(attachee, args, evt_obj):
	print "Destruction Domain Extra Smiting"

	ExtraSmiting = attachee.has_feat("Extra Smiting")
	
	#Extra Smiting grants 2 additional uses of smite each time the feat is taken
	args.set_arg(0, args.get_arg(0) + 2 * ExtraSmiting)
	
	print args.get_arg(0) 
	
	return 0

extendDestructionDomain = PythonModifier()
extendDestructionDomain.ExtendExisting("Destruction Domain")
extendDestructionDomain.AddHook(ET_OnNewDay, EK_NEWDAY_REST, ExtraSmitingNewDayDestructionDomain, ())

def ExtraSmitingNewDaySmiteEvil(attachee, args, evt_obj):
	print "Smite Evil Extra Smiting"

	ExtraSmiting = attachee.has_feat("Extra Smiting")
	
	#Extra Smiting grants 2 additional uses of smite each time the feat is taken
	args.set_arg(0, args.get_arg(0) + 2 * ExtraSmiting)
	return 0

extendSmiteEvil = PythonModifier()
extendSmiteEvil.ExtendExisting("Smite Evil")
extendSmiteEvil.AddHook(ET_OnNewDay, EK_NEWDAY_REST, ExtraSmitingNewDaySmiteEvil, ())
