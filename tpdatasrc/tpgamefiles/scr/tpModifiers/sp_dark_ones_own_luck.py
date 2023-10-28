from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Dark One's Own Luck"

def replaceDarkOnes(attachee, args, evt_obj):
    if (evt_obj.is_modifier("sp-Dark One's Own Luck Fortitude")
    or evt_obj.is_modifier("sp-Dark One's Own Luck Reflex")
    or evt_obj.is_modifier("sp-Dark One's Own Luck Will")):
        args.remove_spell_mod()
        args.remove_spell()
    return 0

darkOnesFortitude = SpellPythonModifier("sp-Dark One's Own Luck Fortitude", 4) # spellId, duration, bonusValue, empty
darkOnesFortitude.AddSaveBonus(passed_by_spell, bonus_type_luck, D20STD_F_NONE, EK_SAVE_FORTITUDE)
darkOnesFortitude.AddHook(ET_OnConditionAddPre, EK_NONE, replaceDarkOnes, ())

darkOnesReflex = SpellPythonModifier("sp-Dark One's Own Luck Reflex", 4) # spellId, duration, bonusValue, empty
darkOnesReflex.AddSaveBonus(passed_by_spell, bonus_type_luck, D20STD_F_NONE, EK_SAVE_REFLEX)
darkOnesReflex.AddHook(ET_OnConditionAddPre, EK_NONE, replaceDarkOnes, ())

darkOnesWill = SpellPythonModifier("sp-Dark One's Own Luck Will", 4) # spellId, duration, bonusValue, empty
darkOnesWill.AddSaveBonus(passed_by_spell, bonus_type_luck, D20STD_F_NONE, EK_SAVE_WILL)
darkOnesWill.AddHook(ET_OnConditionAddPre, EK_NONE, replaceDarkOnes, ())
