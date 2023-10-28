from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Beguiling Influence"

beguilingInfluenceSpell = SpellPythonModifier("sp-Beguiling Influence") # spellId, duration, empty
beguilingInfluenceSpell.AddSkillBonus(6, bonus_type_invocation, skill_bluff, skill_diplomacy, skill_intimidate)
