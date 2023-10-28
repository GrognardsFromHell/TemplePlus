from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Extra Stunning"

# Extra Stunning: Complete Warrior, p. 98

def addExtraCharges(attachee, args, evt_obj):
    featCount = attachee.has_feat("Extra Stunning")
    evt_obj.return_val += 3 * featCount # Extra Stunning adds 3 more charges to Stunning Fist
    return 0

extraStunning = PythonModifier("Extra Stunning Feat", 2) #featEnum, empty
extraStunning.MapToFeat("Extra Stunning", feat_cond_arg2 = 0)
extraStunning.AddHook(ET_OnD20PythonQuery, "PQ_Get_Extra_Stunning_Fist_Charges", addExtraCharges, ())
