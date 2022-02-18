from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Blessing of Bahamut"

blessingOfBahamutSpell = SpellPythonModifier("sp-Blessing of Bahamut") # spell_id, duration, empty
blessingOfBahamutSpell.AddDamageReduction(10, D20DAP_MAGIC)
blessingOfBahamutSpell.AddSpellNoDuplicate() #damage reduction does stack; so I need replaceCondition
