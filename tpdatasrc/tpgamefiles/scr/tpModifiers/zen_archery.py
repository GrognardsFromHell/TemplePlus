from templeplus.pymod import PythonModifier
from toee import *

# Zen Archery: Complete Warrior, p. 106

print "Registering Zen Archery"

def applyWisToHit(attachee, args, evt_obj):
    flags = evt_obj.attack_packet.get_flags()
    if flags & D20CAF_RANGED:
        dexScore = attachee.stat_level_get(stat_dexterity)
        wisScore = attachee.stat_level_get(stat_wisdom)
        dexMod = (dexScore - 10) / 2
        wisMod = (wisScore - 10) / 2
        #To simplify things, just apply the difference between
        #Wisdom and Dexterity as bonus if wis > dex
        if wisMod > dexMod:
            bonus = wisMod - dexMod
            bonusType = 0 #ID 0 = untyped (stacking)
            evt_obj.bonus_list.add(bonus, bonusType, "~Zen Archery~[TAG_ZEN_ARCHERY]")
    return 0

zenArcheryFeat = PythonModifier("Zen Archery Feat", 2) #featEnum, empty
zenArcheryFeat.MapToFeat("Zen Archery", feat_cond_arg2 = 0)
zenArcheryFeat.AddHook(ET_OnToHitBonus2, EK_NONE, applyWisToHit, ())
