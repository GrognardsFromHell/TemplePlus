from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Angelskin"

angelskinSpell = SpellPythonModifier("sp-Angelskin") # spell_id, duration, empty
angelskinSpell.AddDamageReduction(5, D20DAP_UNHOLY)
angelskinSpell.AddSpellNoDuplicate() #damage reduction does stack; so I need replaceCondition
