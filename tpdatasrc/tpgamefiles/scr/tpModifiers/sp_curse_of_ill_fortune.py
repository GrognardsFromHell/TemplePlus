from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Curse of Ill Fortune"

def curseOfIllFortuneSpellPenalty(attachee, args, evt_obj):
    bonusValue = -3 #Curse gives -3 on attack rolls, saves, ability checks and skill checks
    bonusType = 162 #New ID for Curse of Ill Fortune
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Curse of Ill Fortune~[TAG_SPELLS_CURSE_OF_ILL_FORTUNE] penalty")
    return 0

def curseOfIllFortuneSpellCheckRemoveBySpell(attachee, args, evt_obj):
    if evt_obj.is_modifier("sp-Remove Curse"):
        args.remove_spell()
        args.remove_spell_mod()
    return 0

curseOfIllFortuneSpell = PythonModifier("sp-Curse of Ill Fortune", 3, False) # spell_id, duration, empty
curseOfIllFortuneSpell.AddHook(ET_OnToHitBonus2, EK_NONE, curseOfIllFortuneSpellPenalty,())
curseOfIllFortuneSpell.AddHook(ET_OnSaveThrowLevel, EK_NONE, curseOfIllFortuneSpellPenalty,())
curseOfIllFortuneSpell.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE, curseOfIllFortuneSpellPenalty,())
curseOfIllFortuneSpell.AddHook(ET_OnGetSkillLevel, EK_NONE, curseOfIllFortuneSpellPenalty,())
curseOfIllFortuneSpell.AddHook(ET_OnConditionAddPre, EK_NONE, curseOfIllFortuneSpellCheckRemoveBySpell, ())
curseOfIllFortuneSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, (spell_curse_of_ill_fortune,))
curseOfIllFortuneSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, (spell_curse_of_ill_fortune,))
curseOfIllFortuneSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
curseOfIllFortuneSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
curseOfIllFortuneSpell.AddSpellDispelCheckStandard()
curseOfIllFortuneSpell.AddSpellTeleportPrepareStandard()
curseOfIllFortuneSpell.AddSpellTeleportReconnectStandard()
curseOfIllFortuneSpell.AddSpellCountdownStandardHook()
