from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Divine Protection"

divineProtectionSpell = SpellPythonModifier("sp-Divine Protection") # spell_id, duration, empty
divineProtectionSpell.AddAcBonus(1, bonus_type_morale)
divineProtectionSpell.AddSaveBonus(1, bonus_type_morale)
