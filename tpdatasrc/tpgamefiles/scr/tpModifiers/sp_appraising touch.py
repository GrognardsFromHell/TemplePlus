from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Appraising Touch"

def appraisingTouchSpellBonusToAppraise(attachee, args, evt_obj):
    bonusValue = 10 #Appraising Touch is a flat +10 insight bonus to Appraise
    bonusType = 18 #ID 18 = Insight
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Appraising Touch~[TAG_SPELLS_APPRAISING_TOUCH] ~Insight~[TAG_MODIFIER_INSIGHT] Bonus")
    return 0

appraisingTouchSpell = PythonModifier("sp-Appraising Touch", 3, False) # spell_id, duration, empty
appraisingTouchSpell.AddHook(ET_OnGetSkillLevel, EK_SKILL_APPRAISE, appraisingTouchSpellBonusToAppraise, ())
appraisingTouchSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
appraisingTouchSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
appraisingTouchSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
appraisingTouchSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
appraisingTouchSpell.AddSpellDispelCheckStandard()
appraisingTouchSpell.AddSpellTeleportPrepareStandard()
appraisingTouchSpell.AddSpellTeleportReconnectStandard()
appraisingTouchSpell.AddSpellCountdownStandardHook()
