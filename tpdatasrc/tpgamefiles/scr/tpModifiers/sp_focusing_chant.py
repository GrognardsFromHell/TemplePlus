from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Focusing Chant"

def focusingChantSpellBonus(attachee, args, evt_obj):
    bonusValue = 1 #Focusing Chant adds a +1 Circumstance Bonus to Attack Rolls, Skill and Ability Checks
    bonusType = 159 #New ID for Focusing Chant
    evt_obj.bonus_list.add(bonusValue ,bonusType ,"~Circumstance~[TAG_MODIFIER_CIRCUMSTANCE] : ~Focusing Chant~[TAG_SPELLS_FOCUSING_CHANT]")
    return 0

focusingChantSpell = SpellPythonModifier("sp-Focusing Chant") # spellId, duration, empty
focusingChantSpell.AddHook(ET_OnToHitBonus2, EK_NONE, focusingChantSpellBonus,())
focusingChantSpell.AddHook(ET_OnGetSkillLevel, EK_NONE, focusingChantSpellBonus,())
focusingChantSpell.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE, focusingChantSpellBonus,())
