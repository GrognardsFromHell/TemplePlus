#Extend Rage:  Complete Warrior, p. 97

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Extend Rage"

def AddCondition(attachee, args, evt_obj):

	#Add 5 rounds for the extend rage feat
	if attachee.has_feat("Extend Rage"):
		args.set_arg(0, args.get_arg(0) + 5)
	return 0

extendRageFeat = PythonModifier()
extendRageFeat.ExtendExisting("Barbarian_Raged")
extendRageFeat.AddHook(ET_OnConditionAdd, EK_NONE, AddCondition, ())
