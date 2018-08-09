#Craven: Champions of Ruin, p.17

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Craven"

def CADamage(attachee, args, evt_obj):

	#Add 1 damage per Hit Die to the sneak attack
	evt_obj.return_val += attachee.stat_level_get(stat_level)

	return 0

def Fearful(attachee, args, evt_obj):
	if evt_obj.flags & (1<<(D20STD_F_SPELL_DESCRIPTOR_FEAR-1)): # D20STD_F_SPELL_DESCRIPTOR_FEAR
		evt_obj.bonus_list.add(-2, 0, "Craven: You are easily frightened")
	
	return 0
	
cravenFeat = PythonModifier("Craven Feat", 2)
cravenFeat.MapToFeat("Craven")
cravenFeat.AddHook(ET_OnD20PythonQuery, "Sneak Attack Bonus", CADamage, ())
cravenFeat.AddHook(ET_OnSaveThrowLevel, EK_NONE, Fearful, ()) #Generalized even though fear effects should only be will saves.
