from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import spell_utils

print "Spike Growth Damage Extender"

modSpikeGrowthDamage = PythonModifier()
modSpikeGrowthDamage.ExtendExisting("sp-Spike Growth Damage")
modSpikeGrowthDamage.AddHook(ET_OnNewDay, EK_NEWDAY_REST, spell_utils.End, ()) #Should go away on its own after 24 hours




