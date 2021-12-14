from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Fey Heritage: Complete Mage, p. 43

print "Registering Fey Heritage"

def addSavingThrowBonus(attachee, args, evt_obj):
    flags = evt_obj.flags
    if (flags & (1 << (D20STD_F_SPELL_SCHOOL_ENCHANTMENT-1))): 
        bonusValue = 3
        bonusType = 0 # ID 0 = Untyped (stacking)
        evt_obj.bonus_list.add(bonusValue ,bonusType ,"~Fey Heritage~[TAG_FEY_HERITAGE]")
    return 0

def querySelectedHeritage(attachee, args, evt_obj):
    heritage = args.get_arg(1)
    evt_obj.return_val = heritage
    return 0

feyHeritageFeat = PythonModifier("Fey Heritage Feat", 3) #featEnum, heritage, empty
feyHeritageFeat.MapToFeat("Fey Heritage", feat_cond_arg2 = heritage_fey)
feyHeritageFeat.AddHook(ET_OnD20PythonQuery, "PQ_Selected_Heritage", querySelectedHeritage, ())
feyHeritageFeat.AddHook(ET_OnSaveThrowLevel, EK_NONE, addSavingThrowBonus, ())
