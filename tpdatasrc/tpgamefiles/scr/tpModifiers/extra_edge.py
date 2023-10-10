from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import math

#Extra Edge:   Complete Arcane, p. 79

print "Registering Extra Edge"
	
def WarmageEdgeBonusQuery(attachee, args, evt_obj):
	warmageLevel = attachee.stat_level_get(stat_level_warmage)
	bonus = int(math.floor(warmageLevel/4)) + 1
	evt_obj.return_val += bonus
	return 0

warmageEdge = PythonModifier("Extra Edge", 2) # spare, spare
warmageEdge.MapToFeat("Extra Edge")
warmageEdge.AddHook(ET_OnD20PythonQuery, "Warmage Edge Bonus", WarmageEdgeBonusQuery, ())

