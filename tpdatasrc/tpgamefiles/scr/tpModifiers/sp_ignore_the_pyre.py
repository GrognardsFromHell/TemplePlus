from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Ignore the Pyre"

def getElementEnum(elementType):
    mappingDict = {
    D20DT_ACID: ACID,
    D20DT_COLD: COLD,
    D20DT_ELECTRICITY: ELECTRICITY,
    D20DT_FIRE: FIRE,
    D20DT_SONIC: SONIC
    }
    return mappingDict.get(elementType)

def queryHasResist(attachee, args, evt_obj):
    evt_obj.return_val = 1
    #sp-Resist Elements uses descriptors constants
    #I do use damage type constants directly, so I need to remap
    elementType = args.get_arg(3)
    evt_obj.data1 = getElementEnum(elementType)
    return 0

def queryFireballOk(attachee, args, evt_obj):
    resistType = args.get_arg(3)
    if resistType == D20DT_FIRE:
        resistAmount = args.get_arg(2)
        if resistAmount >= 10:
            evt_obj.return_val = 1
    return 0

ignoreThePyreSpell = SpellPythonModifier("sp-Ignore the Pyre", 5) # spellId, duration, bonusValue, elementType, empty
ignoreThePyreSpell.AddDamageResistance(passed_by_spell, passed_by_spell) # bonusValue, elementType
ignoreThePyreSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Resist_Elements, queryHasResist, ())
ignoreThePyreSpell.AddHook(ET_OnD20Query, EK_Q_AI_Fireball_OK, queryFireballOk, ())
ignoreThePyreSpell.AddSpellNoDuplicate()
