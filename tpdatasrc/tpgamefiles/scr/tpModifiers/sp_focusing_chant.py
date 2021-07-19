from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Focusing Chant"

def focusingChantSpellBonus(attachee, args, evt_obj):
    bonusValue = 1 #Focusing Chant adds a +1 Circumstance Bonus to Attack Rolls, Skill and Ability Checks
    bonusType = 159 #New ID for Focusing Chant
    evt_obj.bonus_list.add(bonusValue ,bonusType ,"~Focusing Chant~[TAG_SPELLS_FOCUSING_CHANT] ~Circumstance~[TAG_MODIFIER_CIRCUMSTANCE] Bonus")
    return 0

focusingChantSpell = PythonModifier("sp-Focusing Chant", 3, False) # spell_id, duration, empty
focusingChantSpell.AddHook(ET_OnToHitBonus2, EK_NONE, focusingChantSpellBonus,())
focusingChantSpell.AddHook(ET_OnGetSkillLevel, EK_NONE, focusingChantSpellBonus,())
focusingChantSpell.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE, focusingChantSpellBonus,())
focusingChantSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
focusingChantSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
focusingChantSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
focusingChantSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
focusingChantSpell.AddSpellDispelCheckStandard()
focusingChantSpell.AddSpellTeleportPrepareStandard()
focusingChantSpell.AddSpellTeleportReconnectStandard()
focusingChantSpell.AddSpellCountdownStandardHook()
