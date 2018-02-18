#Vexing Flanker:  Player's Handbook II, p. 85

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Vexing Flanker"

def VFing(attachee, args, evt_obj):

	if attachee.has_feat("Vexing Flanker") != 0:
		#Vexing Flanker
		if evt_obj.attack_packet.get_flags() & D20CAF_FLANKED:
			evt_obj.bonus_list.add(2, 0, "Target Vexing flanker bonus")

	return 0

eVF = PythonModifier("Vexing Flanker Feat", 2)
eVF.MapToFeat("Vexing Flanker")
eVF.AddHook(ET_OnToHitBonus2, EK_NONE, VFing, ())
