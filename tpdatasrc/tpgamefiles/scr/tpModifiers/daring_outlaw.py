from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import math

#Daring Outlaw:   Complete Scoundrel, p. 76

print "Registering Daring Outlaw"
	
def SneakAttackLevelBonus(attachee, args, evt_obj):
	swashLevel = attachee.stat_level_get(stat_level_swashbuckler)
	evt_obj.return_val += swashLevel
	return 0
	
def GraceLevelBonus(attachee, args, evt_obj):
	rogueLevel = attachee.stat_level_get(stat_level_rogue)
	evt_obj.return_val += rogueLevel
	return 0
	
def DodgeLevelBonus(attachee, args, evt_obj):
	rogueLevel = attachee.stat_level_get(stat_level_rogue)
	evt_obj.return_val += rogueLevel
	return 0

daringOutlaw = PythonModifier("Daring Outlaw", 2) # args are just-in-case placeholders
daringOutlaw.MapToFeat("Daring Outlaw")
daringOutlaw.AddHook(ET_OnD20PythonQuery, "Rogue Sneak Attack Level Bonus", SneakAttackLevelBonus, ())
daringOutlaw.AddHook(ET_OnD20PythonQuery, "Swashbuckler Grace Level Bonus", GraceLevelBonus, ())
daringOutlaw.AddHook(ET_OnD20PythonQuery, "Swashbuckler Dodge Level Bonus", DodgeLevelBonus, ())

