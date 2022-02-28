from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Clear Mind"

clearMindSpell = SpellPythonModifier("sp-Clear Mind") # spellId, duration, empty
clearMindSpell.AddSaveBonus(4, bonus_type_sacred, D20STD_F_SPELL_DESCRIPTOR_MIND_AFFECTING)
