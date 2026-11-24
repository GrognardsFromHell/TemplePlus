from templeplus.pymod import PythonModifier
from toee import *
import tpdp

###################################################

def GetConditionName():
	return "Monster Melee Poison Ex"

print("Registering " + GetConditionName())
###################################################

def OnDamage(attachee, args, evt_obj):
	try:
		#print(GetConditionName() + " start...")
		attackNumAllowed = args.get_arg(1) # proto natural attack slot, -1 = any attack
		# proto attacks use codes 1000+slot; natural weapon blend extras (2000+k) map back to their proto slot
		protoIdx = tpdp.get_natural_attack_proto_index(attachee, evt_obj.attack_packet.event_key)
		if ((attackNumAllowed == -1) or (protoIdx == attackNumAllowed)):
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