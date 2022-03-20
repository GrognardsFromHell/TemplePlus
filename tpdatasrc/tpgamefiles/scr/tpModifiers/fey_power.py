from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Fey Power: Complete Mage, p. 43

print "Registering Fey Power"

# Fey Power also increases Warlock Invocation spell level and DC by 1; using isInvocation as Workaround for now

def isInvocation(spellEnum):
    return True if spellEnum in range(spell_eldritch_blast, 2400) else False #2400 needs to be replaced with the last Invocation enum once done

def addBonusDc(attachee, args, evt_obj):
    spellEntry = evt_obj.spell_entry
    spellSchoolEnum = spellEntry.spell_school_enum
    if spellSchoolEnum == Enchantment or isInvocation(spellEntry.spell_enum):
        bonusValue = 1
        bonusType = bonus_type_untyped # Stacking!
        bonusLabel = "~Fey Power~[TAG_FEY_POWER]"
        evt_obj.bonus_list.add(bonusValue, bonusType, bonusLabel)
    return 0

def addBonusCasterLevel(attachee, args, evt_obj):
    spellPacket = evt_obj.get_spell_packet()
    spellEnum = spellPacket.spell_enum
    spellEntry = tpdp.SpellEntry(spellEnum)
    spellSchoolEnum = spellEntry.spell_school_enum
    if spellSchoolEnum == Enchantment or isInvocation(spellEnum):
        evt_obj.return_val += 1
    return 0

feyPowerFeat = PythonModifier("Fey Power Feat", 4) #featEnum, heritage, empty, empty
feyPowerFeat.MapToFeat("Fey Power", feat_cond_arg2 = heritage_fey)
feyPowerFeat.AddHook(ET_OnGetSpellDcMod, EK_NONE, addBonusDc, ())
feyPowerFeat.AddHook(ET_OnGetCasterLevelMod, EK_NONE, addBonusCasterLevel, ())
