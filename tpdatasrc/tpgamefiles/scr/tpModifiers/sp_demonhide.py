from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Demonhide"

def demonhideSpellGrantDr(attachee, args, evt_obj):
    drAmount = 5
    drBreakType = args.get_arg(2)
    damageMesId = 126 #ID126 in damage.mes is DR
    evt_obj.damage_packet.add_physical_damage_res(drAmount, drBreakType, damageMesId)
    return 0

demonhideSpell = PythonModifier("sp-Demonhide", 4, False) # spell_id, duration, drBreakType, empty
demonhideSpell.AddHook(ET_OnTakingDamage, EK_NONE, demonhideSpellGrantDr,())
demonhideSpell.AddHook(ET_OnConditionAddPre, EK_NONE, spell_utils.replaceCondition, ()) #damage reduction does stack; so I need replaceCondition
demonhideSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
demonhideSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
demonhideSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
demonhideSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
demonhideSpell.AddSpellDispelCheckStandard()
demonhideSpell.AddSpellTeleportPrepareStandard()
demonhideSpell.AddSpellTeleportReconnectStandard()
demonhideSpell.AddSpellCountdownStandardHook()
