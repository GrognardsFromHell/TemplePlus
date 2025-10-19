from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import race_utils
import char_class_utils

###################################################

def GetConditionName():
	return "Human"


raceEnum = race_human
raceSpecModule = __import__('race000_human')
###################################################

def OnGetFavoredClass(attachee, args, evt_obj):
	highestLevelClass = char_class_utils.GetHighestBaseClass(attachee)
	if evt_obj.data1 == highestLevelClass:
		evt_obj.return_val = 1
	return 0

def Bonus1(critter, args, evt_obj):
	evt_obj.return_val += 1

	return 0

raceSpecObj = PythonModifier(GetConditionName(), 0)
race_utils.AddBaseMoveSpeed(raceSpecObj, 30)
raceSpecObj.AddHook(ET_OnD20Query, EK_Q_FavoredClass, OnGetFavoredClass, ())
raceSpecObj.AddHook(ET_OnD20PythonQuery, "Bonus Skillpoints", Bonus1, ())
