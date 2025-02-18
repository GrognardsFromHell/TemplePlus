from templeplus.pymod import PythonModifier
from toee import *
from spell_utils import verifyItem
import tpdp


print "Registering masterwork additions"

def Bump(attachee, args, evt_obj):
	armor = evt_obj.obj

	# Apply bonus if item dispatch or armor is worn in the right
	# slot. Null attachee means item dispatch.
	if attachee == OBJ_HANDLE_NULL or verifyItem(armor, args):
		# 12 is enhancement. Prevent stacking with enhancement conditions just
		# in case.
		#
		# 242 is Masterwork
		evt_obj.bonus_list.add(1, 12, 242)

	return 0

# 3 args, arg2 = inventory index for verifyItem
armor_masterwork = PythonModifier() 
armor_masterwork.ExtendExisting("Armor Masterwork")
armor_masterwork.AddHook(ET_OnGetArmorCheckPenalty, EK_NONE, Bump, ())
