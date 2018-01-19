from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils
import d20_action_utils

# Contributed by Winston Shnozwick
print "Registering Psi"

def PsiPoints(attachee, args, evt_obj):
	args.set_arg(0, 0)  # initialize depleted psi points to 0
	args.set_arg(1, 0)  # initialize max psi points to 0
	return 0


def PsiPointsGetMax(attachee, args, evt_obj):
	evt_obj.return_val += args.get_arg(1)
	return 0


def PsiPointsGetCurrent(attachee, args, evt_obj):
	max_psi = attachee.d20_query("Max Psi")
	cur_psi = max_psi - args.get_arg(0)  # current = max - depleted
	if cur_psi > max_psi:  # in case depleted is less than 0
		cur_psi = max_psi
	if cur_psi < 0:  # in case depleted is greater than max
		cur_psi = 0
	evt_obj.return_val = cur_psi
	return 0


def SubtractPsi(attachee, args, evt_obj):
	depleted = args.get_arg(0) + evt_obj.data1
	max_psi = attachee.d20_query("Max Psi")
	if depleted > max_psi:  # in case depleted is greater than max
		depleted = max_psi
	if depleted < 0:  # in case depleted is less than 0
		depleted = 0
	args.set_arg(0, depleted)  # adjust spent points by the amount specified in the event object
	return 0


def PsiPointsNewDay(attachee, args, evt_obj):
	args.set_arg(0, 0)  # set depleted psi points to 0
	# args.set_arg(1, args.get_arg(1)) # set max points to itself, but it never changed!
	# args.set_arg(2, 0) # unused
	# args.set_arg(3, 0) # unused
	return 0


def PsiPointsGetBaseMax(attachee, args, evt_obj):
	evt_obj.return_val = args.get_arg(1)
	return 0

def IncreaseMaxPsi(attachee, args, evt_obj):
	max_psi = attachee.d20_query("Base Max Psi")
	new_max = max_psi + evt_obj.data1
	if new_max < 0: # in case new max is less than 0
		new_max = 0
	args.set_arg(1, new_max) # adjust max points by the amount specified in the event object
	return 0

psiPoints = PythonModifier("Psi Points", 4)  # arg0 - depleted psi points; arg1 - max psi points; arg2 - unused; arg3 - unused
psiPoints.AddHook(ET_OnD20PythonQuery, "Max Psi", PsiPointsGetMax,())  # hook PsiPointsGetCurrent to event of python query
psiPoints.AddHook(ET_OnD20PythonQuery, "Current Psi", PsiPointsGetCurrent,())  # hook PsiPointsGetCurrent to event of python query
psiPoints.AddHook(ET_OnD20PythonSignal, "Subtract Psi", SubtractPsi, ())  # hook SubtractPsi to event of python signal
psiPoints.AddHook(ET_OnNewDay, EK_NEWDAY_REST, PsiPointsNewDay,())  # hook PsiPointsNewDay to event of resting 8 hours safely
psiPoints.AddHook(ET_OnD20PythonQuery, "Base Max Psi", PsiPointsGetBaseMax, ()) # hook PsiPointsGetBaseMax to event of python query
psiPoints.AddHook(ET_OnD20PythonSignal, "Increase Max Psi", IncreaseMaxPsi, ()) # hook IncreaseMaxPsi to event of python signal
