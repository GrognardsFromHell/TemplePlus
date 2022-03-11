from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Curse of Despair"

curseOfDespairSpell = SpellPythonModifier("sp-Curse of Despair") # spellId, duration, empty
curseOfDespairSpell.AddToHitBonus(-1, bonus_type_curse_of_despair)
