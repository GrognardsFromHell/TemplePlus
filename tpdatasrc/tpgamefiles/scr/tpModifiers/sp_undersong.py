from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Undersong"

def undersongSpellReplaceConWithPerform(attachee, args, evt_obj):
    #Undersong replaces Concentration Checks with Perform Checks; to simplify it, add the diff as a generic stacking bonus
    bonusValue = attachee.skill_level_get(skill_perform) - attachee.skill_level_get(skill_concentration)
    bonusType = 157 #New ID for Undersong
    if bonusValue > 0: #Undersong is a may condition
        evt_obj.bonus_list.add(bonusValue, bonusType, "~Undersong~[TAG_SPELLS_UNDERSONG] Bonus")
    return 0

undersongSpell = PythonModifier("sp-Undersong", 3, False) # spell_id, duration, empty
undersongSpell.AddHook(ET_OnGetSkillLevel, EK_SKILL_CONCENTRATION, undersongSpellReplaceConWithPerform,())
undersongSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
undersongSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
undersongSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
undersongSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
undersongSpell.AddSpellDispelCheckStandard()
undersongSpell.AddSpellTeleportPrepareStandard()
undersongSpell.AddSpellTeleportReconnectStandard()
undersongSpell.AddSpellCountdownStandardHook()
