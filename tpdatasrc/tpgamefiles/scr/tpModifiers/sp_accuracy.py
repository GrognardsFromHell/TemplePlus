from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Accuracy"

def doubleIncrement(attachee, args, evt_obj):
    weaponUsed = evt_obj.weapon_used
    evt_obj.range_bonus += weaponUsed.obj_get_int(obj_f_weapon_range)
    return 0

accuracySpell = SpellPythonModifier("sp-Accuracy") # spellId, duration, empty
accuracySpell.AddHook(ET_OnRangeIncrementBonus, EK_NONE, doubleIncrement,())
