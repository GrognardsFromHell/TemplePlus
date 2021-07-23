from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Blessing of Bahamut"

def blessingOfBahamutSpellGrantDr(attachee, args, evt_obj):
    drAmount = 10
    drBreakType = D20DAP_MAGIC
    damageMesId = 126 #ID126 in damage.mes is DR
    evt_obj.damage_packet.add_physical_damage_res(drAmount, drBreakType, damageMesId)
    return 0

blessingOfBahamutSpell = PythonModifier("sp-Blessing of Bahamut", 3, False) # spell_id, duration, empty
blessingOfBahamutSpell.AddHook(ET_OnTakingDamage, EK_NONE, blessingOfBahamutSpellGrantDr,())
blessingOfBahamutSpell.AddHook(ET_OnConditionAddPre, EK_NONE, spell_utils.replaceCondition, ()) #damage reduction does stack; so I need replaceCondition
blessingOfBahamutSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
blessingOfBahamutSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
blessingOfBahamutSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
blessingOfBahamutSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
blessingOfBahamutSpell.AddSpellDispelCheckStandard()
blessingOfBahamutSpell.AddSpellTeleportPrepareStandard()
blessingOfBahamutSpell.AddSpellTeleportReconnectStandard()
blessingOfBahamutSpell.AddSpellCountdownStandardHook()
