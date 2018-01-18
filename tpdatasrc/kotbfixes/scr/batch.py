from toee import *

from __main__ import game

from utilities import *


def partyxpset( xp ):  # CB - sets entire groups experience points to xp
	pc = game.leader
	for obj in pc.group_list():
		curxp = obj.stat_level_get(stat_experience)
		newxp = curxp + xp
		obj.stat_base_set(stat_experience, newxp)
	return 	1
	
def partylevelset(level):  # CB - sets entire groups xp to minimum necessary for level imputted
	pc = game.leader
	for obj in pc.group_list():
		newxp = (level * (500 * (level -1)))
		obj.stat_base_set(stat_experience, newxp)
	return 	1
	
def partyabset (ab, score):  # CB - sets ability to score for entire group
	if (ab == 1):
		abstat = stat_strength
	elif (ab == 2):
		abstat = stat_dexterity
	elif (ab == 3):
		abstat = stat_constitution
	elif (ab == 4):
		abstat = stat_intelligence
	elif (ab == 5):
		abstat = stat_wisdom
	elif (ab == 6):
		abstat = stat_charisma
	else:
		return 0
	if ((score > 0) and (score < 41)):
		pc = game.leader
		for obj in pc.group_list():
			obj.stat_base_set(abstat, score)
	else:
		return 0
	return 1

def massabset(num, score):  # CB - sets all ability scores of specified pc to score
	num = num - 1
	pc = game.party[num]
	if (pc != OBJ_HANDLE_NULL):
		if ((score > 0) and (score < 41)):
			pc.stat_base_set(stat_strength, score)
			pc.stat_base_set(stat_dexterity, score)
			pc.stat_base_set(stat_constitution, score)
			pc.stat_base_set(stat_intelligence, score)
			pc.stat_base_set(stat_wisdom, score)
			pc.stat_base_set(stat_charisma, score)
		else:
			return 0
	else:
		return 0
	return 1
	
def partyhpset(hp):  # CB - sets max hp of entire party to specified value
	pc = game.leader
	for obj in pc.group_list():
		obj.stat_base_set(stat_hp_max, hp)
	return 1

def setbonus( x, type, bonus_one, bonus_two):
	bonus = game.party[x].condition_add_with_args( type, bonus_one, bonus_two )
	return bonus
	
def follower(proto_num):
	npc = game.obj_create(proto_num, game.leader.location)
	if not ( game.leader.follower_atmax() ):
		game.leader.follower_add( npc )
	else:
		game.leader.ai_follower_add( npc )
	
	# add familiar_obj to d20initiative, and set initiative to spell_caster's
	caster_init_value = game.leader.get_initiative()
	npc.add_to_initiative()
	npc.set_initiative( caster_init_value )
	game.update_combat_ui()

def objfset(num, string, y):
	pc = game.party[num]
	return pc.obj_set_int( string, y )

def objfget(num,string):
	pc = game.party[num]
	return pc.obj_get_int(string)

def NighInvulnerable():
	massabset(0, 24)
	necklace = game.obj_create( 6239, game.party[0].location )
	sword1 = game.obj_create( 4599, game.party[0].location )
	sword2 = game.obj_create( 4599, game.party[0].location )
	helm = game.obj_create( 6036, game.party[0].location )
	necklace.item_condition_add_with_args( 'Ring of freedom of movement', 0, 0 )
	necklace.item_condition_add_with_args( 'Amulet of Mighty Fists', 5, 5 )
	necklace.item_condition_add_with_args( 'Weapon Enhancement Bonus', 5, 0 )
	# necklace.item_condition_add_with_args( 'Weapon Holy', 0, 0 )
	# necklace.item_condition_add_with_args( 'Weapon Lawful', 0, 0 )
	necklace.item_condition_add_with_args( 'Weapon Silver', 0, 0 )	
	# necklace.condition_add_with_args( 'Thieves Tools Masterwork', 0, 0)
	# sword.item_condition_add_with_args( 'Weapon Lawful', 0, 0 )
	# sword.item_condition_add_with_args( 'Weapon Silver', 0, 0 )
	game.party[0].item_get(necklace)
	game.party[0].item_get(sword1)
	game.party[0].item_get(sword2)
	game.party[0].item_get(helm)
	game.party[0].condition_add_with_args( 'Monster Regeneration 5', 0, 0 )
	game.party[0].condition_add_with_args( 'Monster Subdual Immunity', 0, 0 )
	game.party[0].condition_add_with_args( 'Monster Energy Immunity', 'Fire', 0 )
	game.party[0].condition_add_with_args( 'Monster Energy Immunity', 'Cold', 0 )
	game.party[0].condition_add_with_args( 'Monster Energy Immunity', 'Electricity', 0 )
	game.party[0].condition_add_with_args( 'Monster Energy Immunity', 'Acid', 0 )
	game.party[0].condition_add_with_args( 'Monster Confusion Immunity', 0, 0 )
	game.party[0].condition_add_with_args( 'Monster Stable', 0, 0 )
	game.party[0].condition_add_with_args( 'Monster Untripable', 0, 0 )
	game.party[0].condition_add_with_args( 'Monster Plant', 0, 0 )
	game.party[0].condition_add_with_args( 'Monster Poison Immunity', 0, 0 )
	game.party[0].condition_add_with_args( 'Saving Throw Resistance Bonus', 0, 10 )
	game.party[0].condition_add_with_args( 'Saving Throw Resistance Bonus',1, 10 )
	game.party[0].condition_add_with_args( 'Saving Throw Resistance Bonus', 2, 10 )
	game.party[0].condition_add_with_args( 'Weapon Holy', 0, 0)
	game.party[0].condition_add_with_args( 'Weapon Lawful', 0, 0)
	game.party[0].condition_add_with_args( 'Weapon Silver', 0, 0)
	game.party[0].obj_set_int(obj_f_speed_run, 2)
	game.party[0].money_adj(+90000000)
	game.particles( 'Orb-Summon-Air-Elemental', game.party[0] )
	return
	
def alldie():
	for obj in game.obj_list_vicinity( game.party[0].location, OLC_CRITTERS ):
		if obj not in game.party and obj.name != 14455:
			obj.critter_kill_by_effect()
			# damage_dice = dice_new( '104d20' )
			# obj.damage( OBJ_HANDLE_NULL, 0, damage_dice )
	return 1

def speedup(ec = 1, file_override = -1):
	if file_override == -1:
		try:
			print 'reading from speedup.ini for speedup preference'
			f = open('modules\\ToEE\\speedup.ini', 'r')
			ec = f.readline()
			print ec
			ec = int(ec)
			f.close()
		except:
			print 'error, using default'
			ec = 1
			print 'ec = ' + str(ec)
	elif file_override != -1:
		ec = file_override
	if ec == 0:
		# Vanilla - 106535216
		for pc in game.party[0].group_list():
			if pc.obj_get_int(obj_f_speed_run) != 1:
				pc.obj_set_int(obj_f_speed_run, 1)
				if game.global_flags[403] == 1:
					game.particles( 'Orb-Summon-Air-Elemental', pc )
	if ec == 1:
		print 'Setting speed to x2'
		# The old value - slightly faster than vanilla (which was 106535216)
		for pc in game.party[0].group_list():
			if pc.obj_get_int(obj_f_speed_run) != 2:
				pc.obj_set_int(obj_f_speed_run, 2)
				if game.global_flags[403] == 1:
					game.particles( 'Orb-Summon-Air-Elemental', pc )
	if ec == 2:
		for pc in game.party[0].group_list():
			if pc.obj_get_int(obj_f_speed_run) != 3:
				pc.obj_set_int(obj_f_speed_run, 3)
				if game.global_flags[403] == 1:
					game.particles( 'Orb-Summon-Air-Elemental', pc )
	if ec == 3:
		for pc in game.party[0].group_list():
			if pc.obj_get_int(obj_f_speed_run) != 4:
				pc.obj_set_int(obj_f_speed_run, 4)
				if game.global_flags[403] == 1:
					game.particles( 'Orb-Summon-Air-Elemental', pc )
	if ec == 4:
		for pc in game.party[0].group_list():
			if pc.obj_get_int(obj_f_speed_run) != 5:
				pc.obj_set_int(obj_f_speed_run, 5)
				if game.global_flags[403] == 1:
					game.particles( 'Orb-Summon-Air-Elemental', pc )
	if ec == 5:
		# Blazingly fast
		for pc in game.party[0].group_list():
			if pc.obj_get_int(obj_f_speed_run) != 6:
				pc.obj_set_int(obj_f_speed_run, 6)
				if game.global_flags[403] == 1:
					game.particles( 'Orb-Summon-Air-Elemental', pc )
	if ec == 6:
		# Can you say speeding bullet?
		for pc in game.party[0].group_list():
			if pc.obj_get_int(obj_f_speed_run) != 10:
				pc.obj_set_int(obj_f_speed_run, 10)
				if game.global_flags[403] == 1:
					game.particles( 'Orb-Summon-Air-Elemental', pc )
	return
