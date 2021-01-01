from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Magic Item Compendium, p. 6

print "Adding Agility Armor and Stamina Armor"

def OnGetSaveBonus(attachee, args, evt_obj):
	armorMes = args.get_param(0)
	value = args.get_arg(0)
		
	BonusString = game.get_mesline("tpmes\\item_creation.mes", 1000 + armorMes)
	evt_obj.bonus_list.add(value, 15, BonusString) #Resistance bonus
	
	return 0

armorAgility = PythonModifier("Armor Agility", 3) # Save Bonus, spare, inv_idx
armorAgility.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveBonus, (231,) ) #Param = First Mes Line


armorStamina = PythonModifier("Armor Stamina", 3) # Save Bonus, spare, inv_idx
armorStamina.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveBonus, (234,) ) #Param = First Mes Line


