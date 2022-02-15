from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Wave of Grief"

spellPenalty = -3

waveOfGriefSpell = SpellPythonModifier("sp-Wave of Grief") # spell_id, duration, empty
waveOfGriefSpell.AddToHitBonus(spellPenalty, bonus_type_wave_of_grief)
waveOfGriefSpell.AddSkillBonus(spellPenalty, bonus_type_wave_of_grief)
waveOfGriefSpell.AddAbilityCheckBonus(spellPenalty, bonus_type_wave_of_grief)
waveOfGriefSpell.AddSaveBonus(spellPenalty, bonus_type_wave_of_grief)
