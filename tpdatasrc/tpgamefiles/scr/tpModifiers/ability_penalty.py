from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from cond_utils import CondPythonModifier
from spell_utils import SpellPythonModifier

print "Registering ability penalty conditions"

# arg0 = duration, particle id, penalty, spare, spare
weakness = CondPythonModifier("Weakness", 5)
weakness.AddAbilityPenalties(43, False, 352, stat_strength)
weakness.AddEndParticles()
weakness.AddRemovedBy('sp-Restoration', 'sp-Greater Restoration')
weakness.AddCancelOut(False, 'sp-Lesser Restoration')

# args: spell_id, duration, penalty, spare, spare
sp_ray_of_clumsiness = SpellPythonModifier("sp-Ray of Clumsiness", 5, tooltip=False)
sp_ray_of_clumsiness.AddAbilityPenalty(0, 12, True, stat_dexterity)
sp_ray_of_clumsiness.AddDispelledBy(False, 'sp-Lesser Restoration')
sp_ray_of_clumsiness.AddRemovedBy('sp-Restoration', 'sp-Greater Restoration')
