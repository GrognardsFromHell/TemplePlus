#Deft Opportunist:  Complete Adventurer, p. 106

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Deft Opportunist"

def DOAOO(attachee, args, evt_obj):

	if attachee.has_feat("Deft Opportunist") != 0:
		#Check if it's an AOO, if so add 4 to the Attack Roll
		if evt_obj.attack_packet.get_flags() & D20CAF_ATTACK_OF_OPPORTUNITY:
			evt_obj.bonus_list.add(4, 0, "Target Deft Opportunist bonus")
			return 0

eDO = PythonModifier("Deft Opportunist Feat", 2)
eDO.MapToFeat("Deft Opportunist")
eDO.AddHook(ET_OnToHitBonus2, EK_NONE, DOAOO, ())
