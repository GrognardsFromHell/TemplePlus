from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Magic Item Compendium, p.6

print "Adding Nimbleness Armor"

def ArmorCheckReduction(attachee, args, evt_obj):
	armor = evt_obj.get_obj_from_args()
	if attachee.item_worn_at(item_wear_armor) == armor:
		evt_obj.return_val = evt_obj.return_val + 2
	return 0
	
def MaxDexBonusIncrease(attachee, args, evt_obj):
	armor = evt_obj.get_obj_from_args()
	if attachee.item_worn_at(item_wear_armor) == armor:
		evt_obj.return_val = evt_obj.return_val + 1
	return 0

armorNimbleness = PythonModifier("Armor Nimbleness", 3) # spare, spare, inv_idx
armorNimbleness.AddHook(ET_OnD20PythonQuery, "Armor Check Penalty Adjustment", ArmorCheckReduction, () )
armorNimbleness.AddHook(ET_OnD20PythonQuery, "Max Dex Bonus Adjustment", MaxDexBonusIncrease, () )


