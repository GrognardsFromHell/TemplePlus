from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Conviction"

convictionSpell = SpellPythonModifier("sp-Conviction", 4) # spell_id, duration, bonusValue, empty
convictionSpell.AddSaveBonus(passed_by_spell, bonus_type_morale)
