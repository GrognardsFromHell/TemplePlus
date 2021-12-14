from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Extra Rage"

#Extra Rage:  Complete Warrior, p. 98

def addExtraCharges(attachee, args, evt_obj):
    # Extra Rage adds 2 more charges to Barbarian Rage
    # every time the feat is taken
    featCount = attachee.has_feat("Extra Rage")
    evt_obj.return_val += 2 * featCount
    return 0

extraRage = PythonModifier("Extra Rage Feat", 2) #featEnum, empty
extraRage.MapToFeat("Extra Rage", feat_cond_arg2 = 0)
extraRage.AddHook(ET_OnD20PythonQuery, "PQ_Get_Extra_Barbarian_Rage_Charges", addExtraCharges, ())
