from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Magic Item Compendium, p.6

print "Adding Quickness Armor"

def OnGetMoveSpeed(attachee, args, evt_obj):
	BonusString = game.get_mesline("tpmes\\item_creation.mes", 1240) #Get Quickness Mes Line
	evt_obj.bonus_list.add(5, 12, BonusString)  #Movement speed increased by 5 feet (enhancement bonus)
	return 0

armorQuickness = PythonModifier("Armor Quickness", 3) # spare, spare, inv_idx
armorQuickness.AddHook(ET_OnGetMoveSpeed, EK_NONE, OnGetMoveSpeed, ())


