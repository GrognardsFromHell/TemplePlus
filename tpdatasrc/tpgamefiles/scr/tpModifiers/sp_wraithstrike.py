from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Wraithstrike"

def negateArmor(attachee, args, evt_obj):
    flags = evt_obj.attack_packet.get_flags()
    if flags & D20CAF_RANGED:
        return 0
    elif flags & D20CAF_TOUCH_ATTACK:
        return 0
    description = "(~Wraithstrike~[TAG_SPELLS_WRAITHSTRIKE]"
    evt_obj.bonus_list.add_cap(9 , 0, 1, description)
    evt_obj.bonus_list.add_cap(10 , 0, 1, description)
    evt_obj.bonus_list.add_cap(12 , 0, 1, description)
    evt_obj.bonus_list.add_cap(28 , 0, 1, description)
    evt_obj.bonus_list.add_cap(29 , 0, 1, description)
    evt_obj.bonus_list.add_cap(33 , 0, 1, description)
    return 0

# Swift spells with a current round duration do not need a duplicate check
wraithstrikeSpell = SpellPythonModifier("sp-Wraithstrike") # spellId, duration, empty
wraithstrikeSpell.AddHook(ET_OnGetAcModifierFromAttacker, EK_NONE, negateArmor,())
