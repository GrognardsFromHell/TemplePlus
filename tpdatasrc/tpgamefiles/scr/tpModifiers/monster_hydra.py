from templeplus.pymod import PythonModifier
from toee import *
import tpdp

def HeadCount(hydra, args, evt_obj):
	evt_obj.return_val = args.get_arg(2)

	return 0

def Yes(hydra, args, evt_obj):
	evt_obj.return_val = 1

	return 0

def Setup(hydra, args, evt_obj):
	# set current heads to initial head count
	args.set_arg(2, args.get_arg(0))

	return 0

def GrowHeads(hydra, args, evt_obj):
	base = args.get_arg(0)
	heads = args.get_arg(2)

	args.set_arg(2, min(base*2, heads + 2))

	return 0

# 0: initial heads
# 1: reserved
# 2: current heads
# 3: extra
# 4: extra
# 5: extra
hydra = PythonModifier('Monster Hydra', 6, 1)
hydra.AddHook(ET_OnConditionAdd, EK_NONE, Setup, ())
hydra.AddHook(ET_OnConditionAddFromD20StatusInit, EK_NONE, Setup, ())
hydra.AddHook(ET_OnD20PythonQuery, 'Hydra Heads', HeadCount, ())
hydra.AddHook(ET_OnD20PythonQuery, 'Full Attack As Standard', Yes, ())
hydra.AddHook(ET_OnD20PythonQuery, 'Full Attack On Charge', Yes, ())
hydra.AddHook(ET_OnD20PythonSignal, 'Hydra Grow Heads', GrowHeads, ())
hydra.AddHook(ET_OnGetCritterNaturalAttacksNum, EK_NONE, HeadCount, ())

def SeverCountdown(hydra, args, evt_obj):
	count = args.get_arg(0)

	if (count <= 0):
		hydra.d20_send_signal('Hydra Grow Heads')
		args.condition_remove()
	else:
		args.set_arg(0, count-1)

	return 0

# condition for tracking a hydra regrowing a severed head.
# currently there's no mechanism for severing a head

# 0: counter
# 1: regrowing
# 2: extra
severed = PythonModifier('Hydra Head Severed', 3, 0)
severed.AddHook(ET_OnBeginRound, EK_NONE, SeverCountdown, ())
