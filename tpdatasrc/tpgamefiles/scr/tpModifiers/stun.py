from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from cond_utils import CondPythonModifier, CountdownRemoveSelf

print "Registering stun condition extender"

def DropHeld(critter, args, evt_obj):
	if not tpdp.config_get_bool("StricterRulesEnforcement"): return 0

	poss = [item_wear_weapon_primary, item_wear_weapon_secondary]

	for pos in poss:
		item = critter.item_worn_at(pos)
		if item == OBJ_HANDLE_NULL: continue

		# TODO: wrong argument passing convention
		# critter.condition_add('Disarmed', item)
		critter.item_worn_unwield(pos, 1) # drop item

	return 0

stun = PythonModifier()
stun.ExtendExisting("Stunned")
# Replaces Q_Helpless; stunned is not helpless
stun.ReplaceHook(2, ET_OnConditionAdd, EK_NONE, DropHeld, ())
# Replaces custom initiative countdown with standard begin round counter
stun.ReplaceHook(4, ET_OnBeginRound, EK_NONE, CountdownRemoveSelf, ())
