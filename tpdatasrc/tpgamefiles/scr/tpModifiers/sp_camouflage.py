from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Camouflage"

def camouflageSpellBonusToHide(attachee, args, evt_obj):
    bonusValue = 10 #Camouflage adds a flat +10 bonus
    bonusType = 158 #New ID for Camouflage
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Camouflage~[TAG_SPELLS_CAMOUFLAGE] ~Circumstance~[TAG_MODIFIER_CIRCUMSTANCE] Bonus")
    return 0

camouflageSpell = PythonModifier("sp-Camouflage", 3, False) # spell_id, duration, empty
camouflageSpell.AddHook(ET_OnGetSkillLevel, EK_SKILL_HIDE, camouflageSpellBonusToHide,())
camouflageSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
camouflageSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
camouflageSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
camouflageSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
camouflageSpell.AddSpellDispelCheckStandard()
camouflageSpell.AddSpellTeleportPrepareStandard()
camouflageSpell.AddSpellTeleportReconnectStandard()
camouflageSpell.AddSpellCountdownStandardHook()
