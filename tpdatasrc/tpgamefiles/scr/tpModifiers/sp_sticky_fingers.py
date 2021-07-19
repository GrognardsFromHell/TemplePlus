from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Sticky Fingers"

def stickyFingersSpellSkillBonus(attachee, args, evt_obj):
    bonusValue = 10 #Sticky Fingers is a flat +10 untyped bouns to Sleight of Hand checks
    bonusType = 156 #New ID for Sticky Fingers
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Sticky Fingers~[TAG_SPELLS_STICKY_FINGERS] Bonus")
    return 0

stickyFingersSpell = PythonModifier("sp-Sticky Fingers", 3) # spell_id, duration, empty
stickyFingersSpell.AddHook(ET_OnGetSkillLevel, EK_SKILL_PICK_POCKET, stickyFingersSpellSkillBonus,())
stickyFingersSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
stickyFingersSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
stickyFingersSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
stickyFingersSpell.AddSpellDispelCheckStandard()
stickyFingersSpell.AddSpellTeleportPrepareStandard()
stickyFingersSpell.AddSpellTeleportReconnectStandard()
stickyFingersSpell.AddSpellCountdownStandardHook()
