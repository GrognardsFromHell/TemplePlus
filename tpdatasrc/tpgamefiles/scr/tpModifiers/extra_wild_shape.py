#Extra Wild Shape:  Complete Divine, p. 81

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Extra Wild Shape"

def ExtraWildShapeQuery(attachee, args, evt_obj):
	
	#Add 2 wild shape uses per feat taken
	featCount = attachee.has_feat("Extra Wild Shape")
	evt_obj.return_val = 2*featCount
	
	return 0
	
def ExtraWildShapeElementalQuery(attachee, args, evt_obj):

	#If over druid level 16, add one use per feat taken
	druidLevel = attachee.stat_level_get(stat_level_druid)
	if druidLevel >= 16:
		featCount = attachee.has_feat("Extra Wild Shape")
		evt_obj.return_val = featCount
		
	return 0

extraWildShapeFeat = PythonModifier("Extra Wild Shape Feat", 2)
extraWildShapeFeat.MapToFeat("Extra Wild Shape")
extraWildShapeFeat.AddHook(ET_OnD20PythonQuery, "Extra Wildshape Uses", ExtraWildShapeQuery, ())
extraWildShapeFeat.AddHook(ET_OnD20PythonQuery, "Extra Wildshape Elemental Uses", ExtraWildShapeElementalQuery, ())
