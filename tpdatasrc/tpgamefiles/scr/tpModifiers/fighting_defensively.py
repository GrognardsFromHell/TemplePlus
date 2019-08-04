from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Fighting Defensively addendum"

#Returns true if the bonus should be active (check box selected and an attack made)
def IsFightingDefensively(attachee, args, evt_obj):

	#The first arg checks the checkbox and the second checks that an attack has been made
	if (args.get_arg(0) != 0) and (args.get_arg(1) != 0):
		evt_obj.return_val = 1
		return 0
	return 0
	
#Only checks that the option is selected but does not require an attack to be made (necessary for attack related feats like deadly defense)
def IsFightingDefensivelyChecked(attachee, args, evt_obj):
	if (args.get_arg(0) != 0):
		evt_obj.return_val = 1
		return 0
	return 0

modExtender = PythonModifier()
modExtender.ExtendExisting("Fighting Defensively")
modExtender.AddHook(ET_OnD20Query, EK_Q_FightingDefensively, IsFightingDefensively, ())
modExtender.AddHook(ET_OnD20PythonQuery, "Fighting Defensively Checked", IsFightingDefensivelyChecked, ())
