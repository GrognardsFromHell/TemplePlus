from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Magic Item Compendium, p. 7

print "Adding Warning Weapon"

def HuntingWeaponDealingDamage(attachee, args, evt_obj):
	target = evt_obj.attack_packet.target
	favored_enemy = attachee.is_favored_enemy(target)
	
	if favored_enemy:
		BonusString = game.get_mesline("tpmes\\item_creation.mes", 1037) #Get Hunting Mes Line
		evt_obj.damage_packet.bonus_list.add(4, 0, BonusString)
	return 0

weaponHunting = PythonModifier("Weapon Hunting", 3) # spare, spare, inv_idx
weaponHunting.AddHook(ET_OnDealingDamage, EK_NONE, HuntingWeaponDealingDamage, ())
