from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Snipers Shot"

def snipersShotSpellEnableSneakAttack(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

snipersShotSpell = PythonModifier("sp-Snipers Shot", 2) # spell_id, duration
snipersShotSpell.AddHook(ET_OnD20PythonQuery, "Disable Sneak Attack Range Requirement", snipersShotSpellEnableSneakAttack,())
snipersShotSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
snipersShotSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
snipersShotSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
snipersShotSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
snipersShotSpell.AddSpellDispelCheckStandard()
snipersShotSpell.AddSpellTeleportPrepareStandard()
snipersShotSpell.AddSpellTeleportReconnectStandard()
snipersShotSpell.AddSpellCountdownStandardHook()
