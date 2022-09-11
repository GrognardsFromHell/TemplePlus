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
	
def towerShieldToHit(attachee, args, evt_obj):

	#Only add the penalty for stricter rules enforcement
	if tpdp.config_get_bool("stricterRulesEnforcement"):
		shield = attachee.item_worn_at(item_wear_shield)
		if shield != OBJ_HANDLE_NULL:
			max_dex_bonus = shield.obj_get_int(obj_f_armor_max_dex_bonus)
			
			# Tower shield check.  Tower shields should have 2 (or slightly more for special materials).
			# Other shields should have a large value (100).
			if max_dex_bonus < 5:
				towerShieldPenalty = -2 # Tower Shield is suppose to have -2 attack penalty
				bonusMesline = 350 # Tower Shield
				evt_obj.bonus_list.add(towerShieldPenalty, 0, bonusMesline)

shieldBonusExtender = PythonModifier()
shieldBonusExtender.ExtendExisting("Shield Bonus")
shieldBonusExtender.AddHook(ET_OnToHitBonus2, EK_NONE, towerShieldToHit, ())
shieldBonusExtender.AddHook(ET_OnGetAC, EK_NONE, MaxDexShield, ())




