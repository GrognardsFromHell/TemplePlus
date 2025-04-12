from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import spell_utils

print "Spike Stones Damage Extender"

modSpikeStonesDamage = PythonModifier()
modSpikeStonesDamage.ExtendExisting("sp-Spike Stones Damage")
modSpikeStonesDamage.AddHook(ET_OnNewDay, EK_NEWDAY_REST, spell_utils.End, ()) #Should go away on its own after 24 hours




