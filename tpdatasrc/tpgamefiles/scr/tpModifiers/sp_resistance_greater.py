from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Resistance Greater"

resistanceGreaterSpell = SpellPythonModifier("sp-Resistance Greater") # spell_id, duration, empty
resistanceGreaterSpell.AddSaveBonus(3, bonus_type_resistance)
