from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Aid Mass"

aidMassSpell = SpellPythonModifier("sp-Aid Mass", 4) # spellId, duration, tempHpAmount, empty
aidMassSpell.AddTempHp(passed_by_spell)
aidMassSpell.AddToHitBonus(1, bonus_type_morale)
aidMassSpell.AddSaveBonus(1, bonus_type_morale, EK_NONE, D20STD_F_SPELL_DESCRIPTOR_FEAR)
