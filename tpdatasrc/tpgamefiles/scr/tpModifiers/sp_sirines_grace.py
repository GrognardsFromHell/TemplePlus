from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Sirines Grace"

### Swim Speed and breath underwater is not applicable in ToEE nor the underwater combat changes ###

sirinesGraceSpell = SpellPythonModifier("sp-Sirines Grace", 4) # spell_id, duration, bonusValue, empty
#Sirines Grace adds a +4 Enhancement Bonus to Charisma and Dexterity
#Sirines Grace adds a +8 untyped Bonus to Perform
#Sirines Grace adds the Charisma Modifier Value as Deflection Bonus to AC
sirinesGraceSpell.AddAbilityBonus(4, bonus_type_enhancement, stat_dexterity, stat_charisma)
sirinesGraceSpell.AddSkillBonus(8, bonus_type_sirines_grace, skill_perform) #to avoid stacking with itself, unique bonus_type
sirinesGraceSpell.AddAcBonus(passed_by_spell, bonus_type_deflection)
