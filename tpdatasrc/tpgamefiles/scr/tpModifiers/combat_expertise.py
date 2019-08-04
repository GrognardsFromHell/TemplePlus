from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Combat Expertise Extender"

def CombatExpertiseValue(attachee, args, evt_obj):
	evt_obj.return_val = args.get_arg(0)
	return 0

powerAttackExtender = PythonModifier()
powerAttackExtender.ExtendExisting("Feat Expertise")
powerAttackExtender.MapToFeat(feat_combat_expertise)
powerAttackExtender.AddHook(ET_OnD20PythonQuery, "Combat Expertise Value", CombatExpertiseValue, ())