from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Curse of Impending Blades"

def curseOfImpendingBladesSpellCheckRemoveBySpell(attachee, args, evt_obj):
    if evt_obj.is_modifier("sp-Remove Curse"):
        args.remove_spell()
        args.remove_spell_mod()
    return 0

curseOfImpendingBladesSpell = SpellPythonModifier("sp-Curse of Impending Blades") # spell_id, duration, empty
curseOfImpendingBladesSpell.AddHook(ET_OnConditionAddPre, EK_NONE, curseOfImpendingBladesSpellCheckRemoveBySpell, ())
curseOfImpendingBladesSpell.AddAcBonus(-2, bonus_type_curse_of_impending_blades)
