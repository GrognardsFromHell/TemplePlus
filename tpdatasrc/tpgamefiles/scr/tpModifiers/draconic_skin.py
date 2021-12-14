from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Draconic Resistance: Races of the Dragon, p. 105

print "Registering Draconic Skin"

def addNaturalArmor(attachee, args, evt_obj):
    bonusValue = 1
    bonusType = 9 #ID 9 = Amulet of Natural Armor
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Draconic Skin~[TAG_DRACONIC_SKIN]")

draconicResistanceFeat = PythonModifier("Draconic Skin Feat", 2) #FeatEnum, empty
draconicResistanceFeat.MapToFeat("Draconic Skin", feat_cond_arg2 = 0)
draconicResistanceFeat.AddHook(ET_OnGetAC, EK_NONE, addNaturalArmor, ())
