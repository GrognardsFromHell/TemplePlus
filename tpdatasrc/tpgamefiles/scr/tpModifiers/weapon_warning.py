from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Magic Item Compendium, p. 7

print "Adding Warning Weapon"

def InitBonusWeaponWarning(attachee, args, evt_obj):
	BonusString = game.get_mesline("tpmes\\item_creation.mes", 1035) #Get Warning Mes Line
	evt_obj.bonus_list.add(5, 0, BonusString) 
	return 0

weaponWarning = PythonModifier("Weapon Warning", 3) # spare, spare, inv_idx
weaponWarning.AddHook(ET_OnGetInitiativeMod, EK_NONE, InitBonusWeaponWarning, ())
