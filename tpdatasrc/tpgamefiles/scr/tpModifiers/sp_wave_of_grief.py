from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Wave of Grief"

def waveOfGriefSpellPenalty(attachee, args, evt_obj):
    bonusValue = -3 #Wave of Grief is a -3 penalty on Attack Rolls, saves and ability and skill checks
    bonusType = 164 #New ID for Wave of Grief
    evt_obj.bonus_list.add(bonusValue ,bonusType, "~Wave of Grief~[TAG_SPELLS_WAVE_OF_GRIEF] Penalty")
    return 0

waveOfGriefSpell = PythonModifier("sp-Wave of Grief", 3, False) # spell_id, duration, empty
waveOfGriefSpell.AddHook(ET_OnToHitBonus2, EK_NONE, waveOfGriefSpellPenalty,())
waveOfGriefSpell.AddHook(ET_OnGetSkillLevel, EK_NONE, waveOfGriefSpellPenalty,())
waveOfGriefSpell.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE, waveOfGriefSpellPenalty,())
waveOfGriefSpell.AddHook(ET_OnSaveThrowLevel, EK_NONE, waveOfGriefSpellPenalty,())
waveOfGriefSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
waveOfGriefSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
waveOfGriefSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
waveOfGriefSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
waveOfGriefSpell.AddSpellDispelCheckStandard()
waveOfGriefSpell.AddSpellTeleportPrepareStandard()
waveOfGriefSpell.AddSpellTeleportReconnectStandard()
waveOfGriefSpell.AddSpellCountdownStandardHook()
