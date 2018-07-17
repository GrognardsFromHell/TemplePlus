#Extra Stunning:  Complete Warrior, p. 98

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Extra Stunning"

def ExtraStunningNewDay(attachee, args, evt_obj):
	#Add 3 extra stunning attacks per feat taken
	featCount = attachee.has_feat("Extra Stunning")
	args.set_arg(0, args.get_arg(0) + 3*featCount)
	return 0

extendRageFeat = PythonModifier()
extendRageFeat.ExtendExisting("feat_stunning_fist")
extendRageFeat.MapToFeat(feat_stunning_fist)
extendRageFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, ExtraStunningNewDay, ())
