from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Nixies Grace"

### Swim Speed and breath underwater is not applicable in ToEE ###

### Is low light vision of revelance? ###

nixiesGraceSpell = SpellPythonModifier("sp-Nixies Grace") # spell_id, duration, empty
nixiesGraceSpell.AddAbilityBonus(6, bonus_type_enhancement, stat_dexterity)
nixiesGraceSpell.AddAbilityBonus(2, bonus_type_enhancement, stat_wisdom)
nixiesGraceSpell.AddAbilityBonus(8, bonus_type_enhancement, stat_charisma)
nixiesGraceSpell.AddDamageReduction(5, D20DAP_COLD)
nixiesGraceSpell.AddSpellNoDuplicate() #damage reduction does stack; so I need replaceCondition
