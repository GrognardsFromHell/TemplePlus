from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Angelskin"

def angelskinSpellEvilDr(attachee, args, evt_obj):
    drAmount = 5
    drBreakType = D20DAP_UNHOLY
    damageMesId = 126 #ID126 in damage.mes is DR
    evt_obj.damage_packet.add_physical_damage_res(drAmount, drBreakType, damageMesId)
    return 0

angelskinSpell = PythonModifier("sp-Angelskin", 3, False) # spell_id, duration, empty
angelskinSpell.AddHook(ET_OnTakingDamage , EK_NONE, angelskinSpellEvilDr,())
angelskinSpell.AddHook(ET_OnConditionAddPre, EK_NONE, spell_utils.replaceCondition, ()) #damage reduction does stack; so I need replaceCondition
angelskinSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
angelskinSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
angelskinSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
angelskinSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
angelskinSpell.AddSpellDispelCheckStandard()
angelskinSpell.AddSpellTeleportPrepareStandard()
angelskinSpell.AddSpellTeleportReconnectStandard()
angelskinSpell.AddSpellCountdownStandardHook()
