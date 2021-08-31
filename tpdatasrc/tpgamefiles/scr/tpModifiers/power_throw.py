#Power Throw:  Complete Adventurer, p. 111

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Power Throw"

def powerThrowDamageBonus(attachee, args, evt_obj):
	if evt_obj.attack_packet.get_flags() & D20CAF_THROWN == 0:
		return 0
		
	#Bonus Value Based on power attack selection
	PowerAttackValue = attachee.d20_query("Power Attack Value")
	
	if PowerAttackValue > 0:
		evt_obj.damage_packet.bonus_list.add_from_feat(PowerAttackValue, 0, 114, "Power Throw")
		
	return 0

def powerThrowAttackBonus(attachee, args, evt_obj):
	if evt_obj.attack_packet.get_flags() & D20CAF_THROWN == 0:
		return 0
		
	#Penalty Value Based on power attack selection
	PowerAttackValue = attachee.d20_query("Power Attack Value")
	
	if PowerAttackValue > 0:
		evt_obj.bonus_list.add_from_feat(-1*PowerAttackValue, 0, 114, "Power Throw")
		
	return 0

powerThrow = PythonModifier("Power Throw", 2) # args are just-in-case placeholders
powerThrow.MapToFeat("Power Throw")
powerThrow.AddHook(ET_OnDealingDamage, EK_NONE, powerThrowDamageBonus, ())
powerThrow.AddHook(ET_OnToHitBonus2, EK_NONE, powerThrowAttackBonus, ())