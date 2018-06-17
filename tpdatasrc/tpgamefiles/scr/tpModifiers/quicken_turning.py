from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#  Quicken Turning:  Complete Divine, p. 84

print "Registering Quicken Turning"

def QuickenTurningBeginRound(attachee, args, evt_obj):
	args.set_arg(0, 0) # reset turn undead used
	return 0

def TurnUndeadDisabled(attachee, args, evt_obj):
	evt_obj.return_val = args.get_arg(0) #Returns 1 if turn undead has already been used
	return 0
	
def TurnUndeadPerform(attachee, args, evt_obj):
	args.set_arg(0, 1) #Set the flag that turn undead has been used this round
	return 0

def QuickenTurningCostMod(attachee, args, evt_obj):
	if evt_obj.d20a.action_type != tpdp.D20ActionType.TurnUndead:
		return 0
		
	# Always a free action with the feat
	evt_obj.cost_new.action_cost = 0

	return 0

quickenTurning = PythonModifier("Quicken Turning Feat", 2) # First argument is whether or not turn undead is used, second is extra
quickenTurning.MapToFeat("Quicken Turning")
quickenTurning.AddHook(ET_OnBeginRound, EK_NONE, QuickenTurningBeginRound, ())
quickenTurning.AddHook(ET_OnD20PythonQuery, "Turn Undead Disabled", TurnUndeadDisabled, ())
quickenTurning.AddHook(ET_OnD20PythonSignal, "Turn Undead Perform", TurnUndeadPerform, ())
quickenTurning.AddHook(ET_OnActionCostMod, EK_NONE, QuickenTurningCostMod, ())
