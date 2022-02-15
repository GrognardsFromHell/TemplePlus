from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Serene Visage"

sereneVisageSpell = SpellPythonModifier("sp-Serene Visage", 4) # spell_id, duration, bonusValue, empty
sereneVisageSpell.AddSkillBonus(passed_by_spell, bonus_type_insight, skill_bluff)
