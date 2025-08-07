from templeplus.pymod import PythonModifier
from toee import *
from spell_utils import verifyItem
import tpdp

print "Registering enhancement additions"

def Bump(attachee, args, evt_obj):
	# Skip if this is a temporary condition from e.g. magic vestment.
	# Armor doesn't become masterwork because a spell has been cast on it.
	if args.get_arg(4) != 0: return 0

	armor = evt_obj.obj

	# Apply bonus if item dispatch or armor is worn in the right
	# slot. Null attachee means item dispatch.
	if attachee == OBJ_HANDLE_NULL or verifyItem(armor, args):
		# 12 is enhancement, to prevent stacking just in case an item gets
		# multiple enhancement/masterwork conditions applied (although crafting
		# actually removes the masterwork condition).
		#
		# 242 is Masterwork mesline
		evt_obj.bonus_list.add(1, 12, 242)

	return 0

# 5 args
# arg0 = amount
# arg2 = inventory index for verifyItem
# arg4 = spell_id
armor_enh = PythonModifier() 
armor_enh.ExtendExisting("Armor Enhancement Bonus")
armor_enh.AddHook(ET_OnGetArmorCheckPenalty, EK_NONE, Bump, ())

shield_enh = PythonModifier()
shield_enh.ExtendExisting("Shield Enhancement Bonus")
shield_enh.AddHook(ET_OnGetArmorCheckPenalty, EK_NONE, Bump, ())
