from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Focusing Chant"

focusingChantSpell = SpellPythonModifier("sp-Focusing Chant") # spellId, duration, empty
#Focusing Chant adds a +1 Circumstance Bonus to Attack Rolls, Skill and Ability Checks
#New ID (159) for Focusing Chant to avoid stacking with itself
focusingChantSpell.AddToHitBonus(1, bonus_type_focusing_chant)
focusingChantSpell.AddSkillBonus(1, bonus_type_focusing_chant, EK_NONE)
focusingChantSpell.AddAbilityCheckBonus(1, bonus_type_focusing_chant)
