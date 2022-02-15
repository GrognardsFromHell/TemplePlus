from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Sticky Fingers"

stickyFingersSpell = SpellPythonModifier("sp-Sticky Fingers") # spell_id, duration, empty
#Sticky Fingers is a flat +10 untyped bouns to Sleight of Hand checks
stickyFingersSpell.AddSkillBonus(10, bonus_type_sticky_fingers) #unique bonus type to avoid stacking with itself
