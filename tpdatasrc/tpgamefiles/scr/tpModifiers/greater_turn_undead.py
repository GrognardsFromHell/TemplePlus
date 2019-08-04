from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Greater Turn Undead extender"

# Fix greater turning so it is usable once per day and uses up a turn undead charge (bug 071).
# SRD:  Once per day, you can perform a greater turning against undead in place of a 
# regular turning. The greater turning is like a normal turning except that the undead 
# creatures that would be turned are destroyed instead.

#Called to deduct a turn undeac charge
def DeductGreaterTurnUndeadCharge(attachee, args, evt_obj):
	NewCharges = args.get_arg(1) - 1
	if NewCharges > 0:
		args.set_arg(1, NewCharges)
	else:
		args.set_arg(1, 0)
	return 0

def OnNewDay(attachee, args, evt_obj):
	args.set_arg(1, 1) #One Charge Per Day
	return 0

modExtender = PythonModifier()
modExtender.ExtendExisting("Greater Turning")
modExtender.AddHook(ET_OnNewDay, EK_NEWDAY_REST, OnNewDay, ())
modExtender.AddHook(ET_OnD20PythonSignal, "Deduct Greater Turn Undead Charge", DeductGreaterTurnUndeadCharge, ())



