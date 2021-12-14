from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Extra Exhalation"

#Extra Exhalation: Races of the Dragon, p. 102

def addExtraCharges(attachee, args, evt_obj):
    featCount = attachee.has_feat("Extra Exhalation")
    evt_obj.return_val += featCount
    return 0

extraExhalation = PythonModifier("Extra Exhalation Feat", 2) #featEnum, empty
extraExhalation.MapToFeat("Extra Exhalation", feat_cond_arg2 = 0)
extraExhalation.AddHook(ET_OnD20PythonQuery, "PQ_Extra_Breath_Charges", addExtraCharges, ())
