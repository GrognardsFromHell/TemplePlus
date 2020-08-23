from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import math

#Swift Ambusher:   Complete Scoundrel, p. 81

print "Registering Swift Ambusher"
	
def SkirmishBonusLevels(attachee, args, evt_obj):
	rogueLevel = attachee.stat_level_get(stat_level_rogue)
	evt_obj.return_val += rogueLevel
	return 0
	

daringOutlaw = PythonModifier("Swift Ambusher", 2) # args are just-in-case placeholders
daringOutlaw.MapToFeat("Swift Ambusher")
daringOutlaw.AddHook(ET_OnD20PythonQuery, "Skrimish Level Bonus", SkirmishBonusLevels, ())

