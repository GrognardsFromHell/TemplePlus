from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Extend wild shape to allow charges to be used for other things
print "Registering wildshape extender"

# Gets the number of wildshape charges
def GetWildshapeCharges(attachee, args, evt_obj):
	#Wildshape charges are the lower bits, elemental charges are the higher bits
	numTimes = args.get_arg(0)
	numTimes = numTimes & 0xFF
	evt_obj.return_val = numTimes
	return 0

# Called to deduct a wild shape charge
def DeductWildshapeCharge(attachee, args, evt_obj):
	#Wildshape charges are the lower bits, elemental charges are the higher bits
	numTimes = args.get_arg(0)
	numTimes = numTimes & 0xFF
	
	#Deduct one regular wild shape charge (make sure there is at least one or things could get really goofed up)
	if numTimes > 0:
		wildshapeValue = args.get_arg(0) - 1
		args.set_arg(0, wildshapeValue)
	return 0
	
def WildshapeConditionAdded(attachee, args, evt_obj):
	#Used by the C++ side to make sure the condition does not get added multiple times
	evt_obj.return_val = 1
	return 0

modExtender = PythonModifier()
modExtender.ExtendExisting("Wild Shaped") #Note:  Arg 0 = number of charges
modExtender.AddHook(ET_OnD20PythonSignal, "Deduct Wild Shape Charge", DeductWildshapeCharge, ())
modExtender.AddHook(ET_OnD20PythonQuery, "Wild Shape Charges", GetWildshapeCharges, ())
modExtender.AddHook(ET_OnD20PythonQuery, "Wild Shaped Condition Added", WildshapeConditionAdded, ())
