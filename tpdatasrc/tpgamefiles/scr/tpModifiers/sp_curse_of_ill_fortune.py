from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Curse of Ill Fortune"

def curseOfIllFortuneSpellCheckRemoveBySpell(attachee, args, evt_obj):
    if evt_obj.is_modifier("sp-Remove Curse"):
        args.remove_spell()
        args.remove_spell_mod()
    return 0

spellPenalty = -3

curseOfIllFortuneSpell = SpellPythonModifier("sp-Curse of Ill Fortune") # spell_id, duration, empty
curseOfIllFortuneSpell.AddHook(ET_OnConditionAddPre, EK_NONE, curseOfIllFortuneSpellCheckRemoveBySpell, ())
curseOfIllFortuneSpell.AddToHitBonus(spellPenalty, bonus_type_curse_of_ill_fortune)
curseOfIllFortuneSpell.AddSaveBonus(spellPenalty, bonus_type_curse_of_ill_fortune)
curseOfIllFortuneSpell.AddSkillBonus(spellPenalty, bonus_type_curse_of_ill_fortune, EK_NONE)
curseOfIllFortuneSpell.AddAbilityCheckBonus(spellPenalty, bonus_type_curse_of_ill_fortune)
