from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Shield Extender"

def MaxDexShield(attachee, args, evt_obj):
	shield = attachee.item_worn_at(item_wear_shield)
	
	if shield != OBJ_HANDLE_NULL:
		max_dex_bonus = shield.get_max_dex_bonus()
	
		if max_dex_bonus > 0:
			capType  = 3 # Effects Dex Bonus
			bonusMesline = 112 # Bonus Reduced by item
			evt_obj.bonus_list.set_cap_with_custom_descr(capType, max_dex_bonus, bonusMesline, shield.description)
	return 0

shieldBonusExtender = PythonModifier()
shieldBonusExtender.ExtendExisting("Shield Bonus")
shieldBonusExtender.AddHook(ET_OnGetAC, EK_NONE, MaxDexShield, ())




