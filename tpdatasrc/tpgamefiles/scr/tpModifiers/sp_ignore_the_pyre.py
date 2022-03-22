from toee import *
import tpdp
from spell_utils import SpellPythonModifier, getElementEnum

print "Registering sp-Ignore the Pyre"

def queryHasResist(attachee, args, evt_obj):
    evt_obj.return_val = 1
    #sp-Resist Elements uses descriptors constants
    #I do use damage type constants directly, so I need to remap
    damageType = args.get_arg(3)
    evt_obj.data1 = getElementEnum(damageType)
    return 0

def queryFireballOk(attachee, args, evt_obj):
    resistType = args.get_arg(3)
    if resistType == D20DT_FIRE:
        resistAmount = args.get_arg(2)
        if resistAmount >= 10:
            evt_obj.return_val = 1
    return 0

ignoreThePyreSpell = SpellPythonModifier("sp-Ignore the Pyre", 5) # spellId, duration, bonusValue, damageType, empty
ignoreThePyreSpell.AddDamageResistance(passed_by_spell, passed_by_spell) # bonusValue, damageType
ignoreThePyreSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Resist_Elements, queryHasResist, ())
ignoreThePyreSpell.AddHook(ET_OnD20Query, EK_Q_AI_Fireball_OK, queryFireballOk, ())
ignoreThePyreSpell.AddSpellNoDuplicate()
