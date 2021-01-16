from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Magic Item Compendium, p.6

print "Adding Mobility Armor"

def MobilityACBonus(attachee, args, evt_obj):
	if evt_obj.attack_packet.get_flags() & D20CAF_AOO_MOVEMENT:
		if not attachee.has_feat(feat_mobility): #Item Grants mobility feat, don't give bonus if the character already has it
			featName = game.get_feat_name(feat_mobility)
			evt_obj.bonus_list.add_from_feat(4, 8, 114, featName)  #Mobility Feat AC Bonus vs AOO
	return 0

armorMobility = PythonModifier("Armor Mobility", 3) # spare, spare, inv_idx
armorMobility.AddHook(ET_OnGetAC, EK_NONE, MobilityACBonus, ())


