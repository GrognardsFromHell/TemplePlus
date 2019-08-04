#Improved Rapid Shot:  Complete Warrior, p. 101

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Improved Rapid Shot"

def impRapidShotHitBonus(attachee, args, evt_obj):
	
	RapidShotEnabled = 0
	RapidShotRangerEnabled = 0
	
	if attachee.has_feat(feat_rapid_shot):
		RapidShotEnabled = attachee.d20_query("Rapid Shot Enabled")
		
	if attachee.has_feat(feat_ranger_rapid_shot):
		RapidShotRangerEnabled = attachee.d20_query("Rapid Shot Ranger Enabled")
	
	#First, rapid shot mode must be enabled either regular or ranger
	if RapidShotEnabled or RapidShotRangerEnabled:
		#Must be a ranged full attack to qualify for the bonus
		if (evt_obj.attack_packet.get_flags() & D20CAF_RANGED) and (evt_obj.attack_packet.get_flags() & D20CAF_FULL_ATTACK):
			evt_obj.bonus_list.add(2, 0, "Improved Rapid Shot Feat") #+2 Bonus makes up for the -2 Rapid shot penalty
	return 0

impRapidShot = PythonModifier("Improved Rapid Shot Feat", 2) # args are just-in-case placeholders
impRapidShot.MapToFeat("Improved Rapid Shot")
impRapidShot.AddHook(ET_OnToHitBonus2, EK_NONE, impRapidShotHitBonus, ())
