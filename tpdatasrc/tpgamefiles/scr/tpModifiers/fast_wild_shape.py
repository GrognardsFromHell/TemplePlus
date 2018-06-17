from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#  Fast Wild Shape:  Complete Divine, p. 81

print "Registering Fast Wild Shape"

def FastWildShapeCostMod(attachee, args, evt_obj):
	if evt_obj.d20a.action_type != tpdp.D20ActionType.ClassAbility:
		return 0
		
	WildshapeValue = 1 << 24
	
	#Check for the wildshape bit
	if (evt_obj.d20a.data1 & WildshapeValue):
		
		#Always a move action with the feat
		evt_obj.cost_new.action_cost = D20ACT_Move_Action

	return 0

fastWildShape = PythonModifier("Fast Wild Shape Feat", 2) # First argument is the wildshape, second is extra
fastWildShape.MapToFeat("Fast Wild Shape")
fastWildShape.AddHook(ET_OnActionCostMod, EK_NONE, FastWildShapeCostMod, ())
