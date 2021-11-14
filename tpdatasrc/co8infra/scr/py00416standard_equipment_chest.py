from toee import *
from utilities import *
from Co8 import *
from py00439script_daemon import npc_set, npc_get
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (npc_get(attachee, 1) == 0):
		triggerer.begin_dialog( attachee, 1 )
	elif (npc_get(attachee, 1) == 1):
		triggerer.begin_dialog( attachee, 100 )
	return SKIP_DEFAULT


def san_start_combat( attachee, triggerer ):
	leader = game.party[0]
	StopCombat(attachee, 0)
	leader.begin_dialog( attachee, 4000 )
	return RUN_DEFAULT
	
	
	
def give_default_starting_equipment(x = 0):
	for pc in game.party:
			if pc.stat_level_get(stat_level_barbarian) > 0:
				for aaa in [4074, 6059, 6011, 6216, 8014]:
					create_item_in_inventory( aaa, pc )
			elif pc.stat_level_get(stat_level_bard) > 0:
				for aaa in [4009, 6147, 6011, 4096 ,5005 ,5005 ,6012 ,6238 ,12564 ,8014]:
					create_item_in_inventory( aaa, pc )	
			elif pc.stat_level_get(stat_level_druid) > 0:
				for aaa in [6216 ,6217 ,4116 ,4115 ,5007 ,5007 ,8014]:
					create_item_in_inventory( aaa, pc )
			elif pc.stat_level_get(stat_level_cleric) > 0 or pc.divine_spell_level_can_cast() > 0:
				for aaa in [6013 ,6011 ,6012 ,6059 ,4071 ,8014]:
					create_item_in_inventory( aaa, pc )	
			elif pc.stat_level_get(stat_level_fighter) > 0:
				for aaa in [6013 ,6010 ,6011 ,6012 ,6059 ,4062 ,8014]:
					create_item_in_inventory( aaa, pc )		
			elif pc.stat_level_get(stat_level_marshal) > 0:
				for aaa in [6013, 6010, 6011, 6012, 6059, 4062, 8014]:
					create_item_in_inventory( aaa, pc )
			elif pc.stat_level_get(stat_level_monk) > 0:
				if pc.stat_level_get(stat_race) in [race_gnome, race_halfling]:
					for aaa in [6205 ,6202 ,4060 ,8014]: # dagger (4060) instead of quarterstaff
						create_item_in_inventory( aaa, pc )	
				else:
					for aaa in [6205 ,6202 ,4110 ,8014]:
						create_item_in_inventory( aaa, pc )	
			elif pc.stat_level_get(stat_level_paladin) > 0:
				for aaa in [6013 ,6012 ,6011 ,6032 ,6059 ,4036 ,6124 ,8014]:
					create_item_in_inventory( aaa, pc )
			elif pc.stat_level_get(stat_level_ranger) > 0:
				for aaa in [6013 ,6012 ,6011 ,6059 ,4049 ,4201 ,5004 ,5004 ,8014 ,6269]:
					create_item_in_inventory( aaa, pc )	
			elif pc.stat_level_get(stat_level_rogue) > 0:
				for aaa in [6042 ,6045 ,6046 ,4049 ,4060 ,6233 ,8014 ,4096 ,5005 ,5005 ,8014 ,12012]:
					create_item_in_inventory( aaa, pc )
			elif pc.stat_level_get(stat_level_swashbuckler) > 0:
				for aaa in [6013 ,6045 ,6046 ,4009 ,4060 ,6238 ,8014]:
					create_item_in_inventory( aaa, pc )
			elif pc.stat_level_get(stat_level_sorcerer) > 0:
				if pc.stat_level_get(stat_race) in [race_gnome, race_halfling]:
					for aaa in [6211 ,6045 ,6046 ,6124 ,4060 ,4115 ,5007 ,5007 ,8014]: # dagger (4060) instead of spear
						create_item_in_inventory( aaa, pc )
				else:
					for aaa in [6211 ,6045 ,6046 ,6124 ,4117 ,4115 ,5007 ,5007 ,8014]:
						create_item_in_inventory( aaa, pc )
			elif pc.stat_level_get(stat_level_warmage) > 0:
				if pc.stat_level_get(stat_race) in [race_gnome, race_halfling]:
					for aaa in [6013 ,6045 ,6046 ,6059, 4071 , 4115 ,5007 ,5007, 8014]:  # mace (4071) instead of spear
						create_item_in_inventory( aaa, pc )
				else:
					for aaa in [6013 ,6045 ,6046 ,6059, 4117 , 4115 ,5007 ,5007, 8014]:
						create_item_in_inventory( aaa, pc )
			elif pc.stat_level_get(stat_level_beguiler) > 0:
				for aaa in [6042 ,6045 ,6046 ,4049 ,4060 ,6233 ,8014 ,4096 ,5005 ,5005 ,8014 ,12012]:
					create_item_in_inventory( aaa, pc )
			elif pc.stat_level_get(stat_level_wizard) > 0 or pc.arcane_spell_level_can_cast() > 0:
				if pc.stat_level_get(stat_race) in [race_gnome, race_halfling]:
					for aaa in [4060 ,4096 ,5005 ,5005 ,6081 ,6143 ,6038 ,6011 ,8014]:
						create_item_in_inventory( aaa, pc )
				else:
					for aaa in [4110 ,4096 ,5005 ,5005 ,6081 ,6143 ,6038 ,6011 ,8014]:
						create_item_in_inventory( aaa, pc )
			elif pc.stat_level_get(stat_level_scout) > 0:
				for aaa in [6013 ,6012 ,6011, 4049, 4201 ,5004 ,5004 ,8014, 6269, 12012]:
					create_item_in_inventory( aaa, pc )
			else: # default to rogue outfit
				for aaa in [6042 ,6045 ,6046 ,4049 ,4060 ,6233 ,8014 ,4096 ,5005 ,5005 ,8014 ,12012]:
					create_item_in_inventory( aaa, pc )
	return
	
def defalt_equipment_autoequip():	
	for pc in game.party:
		pc.item_wield_best_all()