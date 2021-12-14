from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from heritage_feat_utils import countHeritageFeats

# Fey Power: Complete Mage, p. 43

print "Registering Fey Power"

def addBonusDc(attachee, args, evt_obj):
    isEnchantmentSpell = args.get_arg(2)
    if isEnchantmentSpell:
        bonusValue = 1
        bonusType = 0 # ID 0 = Untyped (stacking)
        evt_obj.bonus_list.add(bonusValue, bonusType, "~Fey Power~[TAG_FEY_POWER]")
        args.set_arg(2, 0)
    return 0

def addBonusCasterLevel(attachee, args, evt_obj):
    spellPacket = evt_obj.get_spell_packet()
    spellEnum = spellPacket.spell_enum
    spellEntry = tpdp.SpellEntry(spellEnum)
    spellSchoolEnum = spellEntry.spell_school_enum
    if spellSchoolEnum == Enchantment:
        evt_obj.return_val += 1
        args.set_arg(2, 1)
    return 0

feyPowerFeat = PythonModifier("Fey Power Feat", 4) #featEnum, heritage, isEnchantmentSpellFlag
feyPowerFeat.MapToFeat("Fey Power", feat_cond_arg2 = heritage_fey)
feyPowerFeat.AddHook(ET_OnGetSpellDcMod, EK_NONE, addBonusDc, ())
feyPowerFeat.AddHook(ET_OnGetCasterLevelMod, EK_NONE, addBonusCasterLevel, ())
