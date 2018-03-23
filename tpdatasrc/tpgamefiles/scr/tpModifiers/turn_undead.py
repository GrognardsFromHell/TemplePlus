from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Turn Undead extender"

def GetTurnUndeadCharges(attachee, args, evt_obj):
	evt_obj.return_val = args.get_arg(1)
	return 0

def DeductTurnUndeadCharge(attachee, args, evt_obj):
	#Deduct one turn undead charge
	NewCharges = args.get_arg(1) - 1
	if NewCharges > 0:
		args.set_arg(1, NewCharges)
	else:
		args.set_arg(1, 0)
	return 0

modExtender = PythonModifier()
modExtender.ExtendExisting("Turn Undead")
modExtender.AddHook(ET_OnD20PythonSignal, "Deduct Turn Undead Charge", DeductTurnUndeadCharge, ())
modExtender.AddHook(ET_OnD20PythonQuery, "Turn Undead Charges", GetTurnUndeadCharges, ())



