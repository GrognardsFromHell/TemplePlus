from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Camouflage"

camouflageSpell = SpellPythonModifier("sp-Camouflage") # spell_id, duration, empty
#Camouflage adds a flat +10 bonus; new bonus_type ID to avoid stacking with itself
camouflageSpell.AddSkillBonus(10, bonus_type_camouflage, skill_hide)
