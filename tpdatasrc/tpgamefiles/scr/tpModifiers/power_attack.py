from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Power Attack Extender"

def PowerAttackValue(attachee, args, evt_obj):
	evt_obj.return_val = args.get_arg(0)
	return 0

powerAttackExtender = PythonModifier()
powerAttackExtender.ExtendExisting("Power Attack")
powerAttackExtender.MapToFeat(feat_power_attack)
powerAttackExtender.AddHook(ET_OnD20PythonQuery, "Power Attack Value", PowerAttackValue, ())