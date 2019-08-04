from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Turn Undead extender"

# Called when a turn undead is performed:  The data is the turn undead type
def TurnUndeadPerform(attachee, args, evt_obj):
	if evt_obj.data1 == 7:
		#Greater Turning:  Also deduct a turn undead charge
		NewCharges = args.get_arg(1) - 1
		if NewCharges > 0:
			args.set_arg(1, NewCharges)
		else:
			args.set_arg(1, 0)
	elif evt_obj.data1 == 0:
		#Standard Turn Undead
		charges = args.get_arg(1)
		if charges == 0:
			attachee.d20_send_signal("Deduct Greater Turn Undead Charge") #Zero out greater turning
	return 0

# Gets the number of turn undead charges
def GetTurnUndeadCharges(attachee, args, evt_obj):
	evt_obj.return_val = args.get_arg(1)
	return 0

# Called to deduct a turn undead charge
def DeductTurnUndeadCharge(attachee, args, evt_obj):
	#Deduct one turn undead charge
	NewCharges = args.get_arg(1) - 1
	if NewCharges > 0:
		args.set_arg(1, NewCharges)
	else:
		args.set_arg(1, 0)
		attachee.d20_send_signal("Deduct Greater Turn Undead Charge") #Zero out greater turning
	return 0

modExtender = PythonModifier()
modExtender.ExtendExisting("Turn Undead") #Note:  Arg 0 = turn undead type, Arg 1 =  is the number of chrages
modExtender.AddHook(ET_OnD20PythonSignal,"Turn Undead Perform", TurnUndeadPerform, ())
modExtender.AddHook(ET_OnD20PythonSignal, "Deduct Turn Undead Charge", DeductTurnUndeadCharge, ())
modExtender.AddHook(ET_OnD20PythonQuery, "Turn Undead Charges", GetTurnUndeadCharges, ())



