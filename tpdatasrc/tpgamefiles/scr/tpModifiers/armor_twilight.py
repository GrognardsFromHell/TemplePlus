from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Armor Twilight"


def TwilightSpellFailure(attachee, args, evt_obj):
	is_valid = False
	inv_idx = args.get_arg(2)
	if inv_idx == -1: # inventory tooltip query
		is_valid = True
	elif inv_idx == 205: # worn item query in ArmorSpellFailure callback
		equip_slot = evt_obj.data2
		if equip_slot == item_wear_armor:
			is_valid = True
	
	if is_valid:
		evt_obj.return_val += -10
	return 0

armorTwilight = PythonModifier("Armor Twilight", 3) # spare, spare, inv_idx
armorTwilight.AddHook(ET_OnD20Query, EK_Q_Get_Arcane_Spell_Failure, TwilightSpellFailure, ())


