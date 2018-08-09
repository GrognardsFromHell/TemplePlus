#Favored Power Attack:  Complete Warrior, p. 98

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Favored Power Attack"

def favoredPowerAttackDamageBonus(attachee, args, evt_obj):
	weapon = evt_obj.attack_packet.get_weapon_used()
	wieldType = attachee.get_wield_type(weapon)
	weap_flags = weapon.obj_get_int(obj_f_weapon_flags)
	
	#No Power attack for light weapons or ranged weapons
	if (wieldType != 0) and not(weap_flags & OWF_RANGED_WEAPON):
	
		target = evt_obj.attack_packet.target
		favored_enemy = attachee.is_favored_enemy(target)
		
		if favored_enemy:
			#Bonus Value Based on power attack selection
			PowerAttackValue = attachee.d20_query("Power Attack Value")
			
			if PowerAttackValue > 0:
				#Add 1x more power attack for one or two handed for a total of x2 or x3 damage
				evt_obj.damage_packet.bonus_list.add_from_feat(PowerAttackValue, 0, 114, "Favored Power Attack")
	return 0

favoredPowerAttack = PythonModifier("Favored Power Attack", 2) # args are just-in-case placeholders
favoredPowerAttack.MapToFeat("Favored Power Attack")
favoredPowerAttack.AddHook(ET_OnDealingDamage, EK_NONE, favoredPowerAttackDamageBonus, ())
