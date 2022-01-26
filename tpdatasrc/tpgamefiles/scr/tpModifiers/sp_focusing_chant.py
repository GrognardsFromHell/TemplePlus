from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier, getSpellHelpTag

print "Registering sp-Focusing Chant"

def focusingChantSpellBonus(attachee, args, evt_obj):
    bonusValue = 1 #Focusing Chant adds a +1 Circumstance Bonus to Attack Rolls, Skill and Ability Checks
    bonusType = bonus_type_focusing_chant #New ID (159) for Focusing Chant to avoid stacking with itself
    bonusHelpTag = game.get_mesline("mes\\bonus_description.mes", bonusType)
    spellId = args.get_arg(0)
    spellHelpTag = getSpellHelpTag(spellId)
    evt_obj.bonus_list.add(bonusValue, bonusType, "{} : {}".format(bonusHelpTag, spellHelpTag))
    return 0

focusingChantSpell = SpellPythonModifier("sp-Focusing Chant") # spellId, duration, empty
focusingChantSpell.AddHook(ET_OnToHitBonus2, EK_NONE, focusingChantSpellBonus,())
focusingChantSpell.AddHook(ET_OnGetSkillLevel, EK_NONE, focusingChantSpellBonus,())
focusingChantSpell.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE, focusingChantSpellBonus,())
