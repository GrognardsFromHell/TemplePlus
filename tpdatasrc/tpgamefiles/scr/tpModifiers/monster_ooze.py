from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Monster Ooze Extender"

def OozeNoTripQuery(attachee, args, evt_obj):
	evt_obj.return_val = 1
	return 0

modMonsterOoze = PythonModifier()
modMonsterOoze.ExtendExisting("Monster Ooze")
modMonsterOoze.AddHook(ET_OnD20Query, EK_Q_Untripable, OozeNoTripQuery, ())





