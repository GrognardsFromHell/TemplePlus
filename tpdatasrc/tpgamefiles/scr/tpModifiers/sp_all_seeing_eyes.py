from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-All Seeing Eyes"

beguilingInfluenceSpell = SpellPythonModifier("sp-All Seeing Eyes") # spellId, duration, empty
beguilingInfluenceSpell.AddSkillBonus(6, bonus_type_invocation, skill_search, skill_spot)
