from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Righteous Fury"

righteousFurySpell = SpellPythonModifier("sp-Righteous Fury", 4) # spellId, duration, tempHpAmount, empty
#Temporary_Hit_Points duration would be 1 hour, but I do not think
#It's worth the trouble to seperate the duration from the spell
righteousFurySpell.AddTempHp(passed_by_spell)
righteousFurySpell.AddAbilityBonus(4, bonus_type_sacred, stat_strength)
