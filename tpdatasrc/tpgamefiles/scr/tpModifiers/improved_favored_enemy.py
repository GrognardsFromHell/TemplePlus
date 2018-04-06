#Improved Favored Enemy:  Complete Warrior, p. 101

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Improved Favored Enemy"

#Feat adds 3 extra damage
bon_val = 3

def impFavoredEnemyDamageBonus(attachee, args, evt_obj):
		
	target = evt_obj.attack_packet.target
	favored_enemy = attachee.is_favored_enemy(target)
	
	if favored_enemy:
		evt_obj.damage_packet.bonus_list.add_from_feat(bon_val, 0, 114, "Improved Favored Enemy")
	return 0

impFavoredEnemy = PythonModifier("Improved Favored Enemy", 2) # args are just-in-case placeholders
impFavoredEnemy.MapToFeat("Improved Favored Enemy")
impFavoredEnemy.AddHook(ET_OnDealingDamage, EK_NONE, impFavoredEnemyDamageBonus, ())
