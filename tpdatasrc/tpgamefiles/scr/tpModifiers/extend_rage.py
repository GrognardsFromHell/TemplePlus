from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Extend Rage:  Complete Warrior, p. 97

print "Registering Extend Rage"

def extendRageDuration(attachee, args, evt_obj):
    #Add 5 rounds for the extend rage feat
    evt_obj.return_val += 5
    return 0

extendRageFeat = PythonModifier("Extend Rage", 2) #featEnum, empty
extendRageFeat.MapToFeat("Extend Rage", feat_cond_arg2 = 0)
extendRageFeat.AddHook(ET_OnD20PythonQuery, "PQ_Extend_Barbarian_Rage", extendRageDuration, ())
