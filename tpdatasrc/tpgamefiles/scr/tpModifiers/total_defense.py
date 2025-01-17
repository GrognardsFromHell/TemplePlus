from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Total Defense extender"

#The fighting defensively query needs to cover both fighting defensively and total defense
def FightingDefensivelyQuery(attachee, args, evt_obj):
	#If args.get_arg(0)clause removed by Sagenlicht
	#as Total Defense has no args and resulted in an error with the newly added Python Query
	#Total Defense is a condition, that will be only added, if the action is actually triggered
	#So the query only triggers if the action is activated anyways.
	evt_obj.return_val = 1
	return 0
	
# No making AOOs when using total defense
def TotalDefenseAOOPossible(attachee, args, evt_obj):
	#If clause added by Sagenlicht to allow AoO's if character has
	#the Active Shield Defense Feat
	if not attachee.has_feat("Active Shield Defense"):
		evt_obj.return_val = 0
	return 0

modExtender = PythonModifier()
modExtender.ExtendExisting("Total Defense")
modExtender.AddHook(ET_OnD20Query, EK_Q_FightingDefensively, FightingDefensivelyQuery, ())
modExtender.AddHook(ET_OnD20Query, EK_Q_AOOPossible, TotalDefenseAOOPossible, ())
#PythonQuery added by Sagenlicht, as Active Shield Defense needs
#A query that is only responding to Total Defense
modExtender.AddHook(ET_OnD20PythonQuery, "PQ_Total_Defense_Activated", FightingDefensivelyQuery, ())
