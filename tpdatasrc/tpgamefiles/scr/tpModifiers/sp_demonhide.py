from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Demonhide"

def demonhideSpellGrantDr(attachee, args, evt_obj):
    drAmount = 5
    drBreakType = args.get_arg(2)
    damageMesId = 126 #ID126 in damage.mes is DR
    evt_obj.damage_packet.add_physical_damage_res(drAmount, drBreakType, damageMesId)
    return 0

demonhideSpell = SpellPythonModifier("sp-Demonhide", 4) # spell_id, duration, drBreakType, empty
demonhideSpell.AddHook(ET_OnTakingDamage2, EK_NONE, demonhideSpellGrantDr,())
demonhideSpell.AddSpellNoDuplicate() #damage reduction does stack; so I need replaceCondition
