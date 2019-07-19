from templeplus.pymod import PythonModifier
from toee import *

###################################################

def GetConditionName():
	return "Monster Melee Poison Ex"

print("Registering " + GetConditionName())
###################################################

def OnDamage(attachee, args, evt_obj):
	try:
		#print(GetConditionName() + " start...")
		attackNumAllowed = args.get_arg(1)
		attackNumAllowed = attackNumAllowed + 1000
		#print(GetConditionName() + " attackNumAllowed (+1000): " + str(attackNumAllowed))
		#print(GetConditionName() + " evt_obj.attack_packet.event_key: " + str(evt_obj.attack_packet.event_key))
		if ((attackNumAllowed == 999) or (evt_obj.attack_packet.event_key == attackNumAllowed)):
			poisonId = args.get_arg(0)
			#print(GetConditionName() + " poisonId: " + str(poisonId))
			#print(GetConditionName() + " adding condition Poisoned...")
			evt_obj.attack_packet.target.condition_add_with_args("Poisoned",poisonId,0)
		else:
			print(GetConditionName() + " skip poison due to wrong attack")
	except Exception as e:
		print(getattr(e, 'message', repr(e)))
		print(getattr(e, 'message', str(e)))
	return 0


modObj = PythonModifier(GetConditionName(), 2)
modObj.AddHook(ET_OnDealingDamage2, EK_NONE, OnDamage, ())