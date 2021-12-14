from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from heritage_feat_utils import countHeritageFeats

# Fey Skin: Complete Mage, p. 43

print "Registering Fey Skin"

def addDamageReduction(attachee, args, evt_obj):
    heritage = args.get_arg(1)
    numberOfFeyFeats = countHeritageFeats(attachee, heritage)
    drAmount = numberOfFeyFeats
    drBreakType = D20DAP_COLD # Cold Iron
    damageMesId = 126 #ID126 in damage.mes is DR
    evt_obj.damage_packet.add_physical_damage_res(drAmount, drBreakType, damageMesId)
    return 0

feySkinFeat = PythonModifier("Fey Skin Feat", 3) #featEnum, heritage, empty
feySkinFeat.MapToFeat("Fey Skin", feat_cond_arg2 = heritage_fey)
feySkinFeat.AddHook(ET_OnTakingDamage2, EK_NONE, addDamageReduction, ())
