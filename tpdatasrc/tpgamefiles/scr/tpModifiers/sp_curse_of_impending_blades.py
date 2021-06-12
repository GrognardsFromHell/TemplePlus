from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Curse of Impending Blades"

def curseOfImpendingBladesSpellPenaltyToAc(attachee, args, evt_obj):
    bonusValue = -2 #Curse of Impending Blades is a -2 penalty to AC
    bonusType = 0 #ID 0 = Untyped (stacking)
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Curse of Impending Blades~[TAG_SPELLS_CURSE_OF_IMPENDING_BLADES] penalty")
    return 0

def curseOfImpendingBladesSpellCheckRemoveBySpell(attachee, args, evt_obj):
    #Limited Wish, Miracle and Wish also remove Curses
    if (evt_obj.is_modifier("sp-Break Enchantment")
    or evt_obj.is_modifier("sp-Remove Curse")):
        args.remove_spell()
        args.remove_spell_mod()
    return 0

curseOfImpendingBladesSpell = PythonModifier("sp-Curse of Impending Blades", 2) # spell_id, duration
curseOfImpendingBladesSpell.AddHook(ET_OnGetAC, EK_NONE, curseOfImpendingBladesSpellPenaltyToAc,())
curseOfImpendingBladesSpell.AddHook(ET_OnConditionAddPre, EK_NONE, curseOfImpendingBladesSpellCheckRemoveBySpell, ())
curseOfImpendingBladesSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, (spell_curse_of_impending_blades,))
curseOfImpendingBladesSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, (spell_curse_of_impending_blades,))
curseOfImpendingBladesSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
curseOfImpendingBladesSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
curseOfImpendingBladesSpell.AddSpellTeleportPrepareStandard()
curseOfImpendingBladesSpell.AddSpellTeleportReconnectStandard()
curseOfImpendingBladesSpell.AddSpellCountdownStandardHook()
