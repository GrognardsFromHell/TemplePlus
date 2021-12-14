from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from heritage_feat_utils import getDraconicHeritageElement
import feat_utils

# Draconic Power: Complete Arcane, p. 78
# Draconic Power: Races of the Dragon, p. 104

print "Registering Draconic Power"

def getSaveDescriptor(heritageElement):
    if heritageElement == D20DT_ACID:
        return 0x1 #D20SPELL_DESCRIPTOR_ACID
    elif heritageElement == D20DT_COLD:
        return 0x4 #D20SPELL_DESCRIPTOR_COLD
    elif heritageElement == D20DT_ELECTRICITY:
        return 0x20 #D20SPELL_DESCRIPTOR_ELECTRICITY
    elif heritageElement == D20DT_FIRE:
        return 0x100 #D20SPELL_DESCRIPTOR_FIRE
    return 0

def checkSpellDescriptor(heritage, spellEntry):
    heritageElement = getDraconicHeritageElement(heritage)
    saveDescriptor = getSaveDescriptor(heritageElement)
    descriptorFlags = spellEntry.descriptor
    if descriptorFlags & saveDescriptor:
        return True
    return False

def addBonusCasterLevel(attachee, args, evt_obj):
    heritage = attachee.d20_query("PQ_Selected_Draconic_Heritage")
    spellPacket = evt_obj.get_spell_packet()
    spellEnum = spellPacket.spell_enum
    spellEntry = tpdp.SpellEntry(spellEnum)
    if checkSpellDescriptor(heritage, spellEntry):
        evt_obj.return_val += 1
    return 0

def addDcBonus(attachee, args, evt_obj):
    heritage = attachee.d20_query("PQ_Selected_Draconic_Heritage")
    spellEntry = evt_obj.spell_entry
    if checkSpellDescriptor(heritage, spellEntry):
        featName = feat_utils.getFeatName(args)
        featTag = feat_utils.getFeatTag(featName)
        bonusValue = 1
        bonusType = 0 # ID 0 = Untyped (stacking)
        evt_obj.bonus_list.add(bonusValue ,bonusType ,"~{}~[{}]".format(featName, featTag))
    return 0

draconicPowerFeat = PythonModifier("Draconic Power", 2) #featEnum, empty
draconicPowerFeat.MapToFeat("Draconic Power", feat_cond_arg2 = 0)
draconicPowerFeat.AddHook(ET_OnGetCasterLevelMod, EK_NONE, addBonusCasterLevel, ())
draconicPowerFeat.AddHook(ET_OnGetSpellDcMod, EK_NONE, addDcBonus, ())
