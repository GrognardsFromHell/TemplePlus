from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Leaps and Bounds"

beguilingInfluenceSpell = SpellPythonModifier("sp-Leaps and Bounds") # spellId, duration, empty
beguilingInfluenceSpell.AddSkillBonus(6, bonus_type_invocation, skill_balance, skill_jump, skill_tumble)
