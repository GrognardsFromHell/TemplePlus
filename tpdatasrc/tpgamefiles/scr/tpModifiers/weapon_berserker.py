from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Magic Item Compendium, p. 7

print "Adding Berserker Weapon"

def DamageBonusWeaponBerserker(attachee, args, evt_obj):
	if (attachee.d20_query(Q_Barbarian_Raged) == 1):
		damage_dice = dice_new('1d8') #1d8 bonus damage
		evt_obj.damage_packet.add_dice(damage_dice, -1, 100) #Damage Type = weapon
	return 0

weaponBerserker = PythonModifier("Weapon Berserker", 3) # spare, spare, inv_idx
weaponBerserker.AddHook(ET_OnDealingDamage, EK_NONE, DamageBonusWeaponBerserker, ())
