from templeplus.pymod import PythonModifier
from toee import *
from spell_utils import verifyItem
import tpdp

#Magic Item Compendium, p.6

print "Adding Nimbleness Armor"

def ArmorCheckReduction(attachee, args, evt_obj):
	armor = evt_obj.obj

	# null attachee means item dispatch
	if attachee == OBJ_HANDLE_NULL or verifyItem(armor, args):
		evt_obj.bonus_list.add(2, 0, "Nimbleness")

	return 0
	
def MaxDexBonusIncrease(attachee, args, evt_obj):
	armor = evt_obj.obj

	# null attachee means item dispatch
	if attachee == OBJ_HANDLE_NULL or verifyItem(armor, args):
		evt_obj.bonus_list.add(1, 0, "Nimbleness")

	return 0

armorNimbleness = PythonModifier("Armor Nimbleness", 3) # spare, spare, inv_idx
armorNimbleness.AddHook(ET_OnGetArmorCheckPenalty, EK_NONE, ArmorCheckReduction, () )
armorNimbleness.AddHook(ET_OnGetMaxDexAcBonus, EK_NONE, MaxDexBonusIncrease, () )


