from toee import *

from __main__ import game

from utilities import *


# game.global_flags[403] - Test Mode flag
# Enables float message (which indicate chosen strategy, etc)


##########################################################################################
###  these are scripting routines I use all the time					##
###  note that I didn't write them all I just use them and modify them as needed	##
###  	- Livonya									##
##########################################################################################


	
##########################################################################################
## This tags and destroys the NPC's ranged weapon, melee weapon and shield		##	
## As a fail safe, if you run this script with an NPC that has muliple weapons		##	
## The extra weapons will vanish							##
## obj_f_pad_i_7 records the type of thrown weapon					##
## obj_f_pad_i_9 records the type of melee weapon					##
## obj_f_pad_i_8 records quantity of thrown weapon					##
##########################################################################################

def tag_weapons(npc):
	npc.item_wield_best_all()				## equip something
	weaponx = npc.item_worn_at(3)				## find weapon being used
	while(weaponx != OBJ_HANDLE_NULL):			## start loop
		npc.item_wield_best_all()			## equip something
		shieldx = npc.item_worn_at(11)			## find shield being used
		if shieldx != OBJ_HANDLE_NULL:			## was a shield found?
			shield = shieldx.name			## get shield's protos name
			npc.obj_set_int( obj_f_pad_i_8, shield )## record shield's name
			shieldx.destroy()			## destroy shield
			npc.item_wield_best_all()		## equip something
		x1 = weaponx.obj_get_int( obj_f_weapon_flags )	## check weapon flag type
		if (x1 == 1024):				## if weapon is ranged
			weapon = weaponx.name			## get weapon's protos name
			npc.obj_set_int( obj_f_pad_i_7, weapon )## record ranged weapon's name
			weaponx.destroy()			## destroy ranged weapon
		else:						## if weapon is not ranged
			weapon = weaponx.name			## get weapon's protos name
			npc.obj_set_int( obj_f_pad_i_9, weapon )## record melee weapon's name
			weaponx.destroy()			## destroy melee weapon	
		npc.item_wield_best_all()			## equip something
		weaponx = npc.item_worn_at(3)			## find weapon being used
	return


##########################################################################################
## This recalls all weapons and shield, and resets variables to 0			##	
##########################################################################################

def get_everything(npc): 
	weapon1 = npc.obj_get_int( obj_f_pad_i_9 )		## recall melee weapon
	weapon2 = npc.obj_get_int( obj_f_pad_i_7 )		## recall ranged weapon
	shield = npc.obj_get_int( obj_f_pad_i_8 )		## recall shield
	if weapon1 == 0 and weapon2 == 0 and shield == 0:	## nothing set?
		return						## end routine
	weaponx1 = npc.item_find_by_proto(weapon1)		## does melee weapon already exist?
	if weapon1 != 0 and weaponx1 == OBJ_HANDLE_NULL:	## if melee weapons doesn't exist then...
		create_item_in_inventory( weapon1, npc )	## create melee weapon
	weaponx2 = npc.item_find_by_proto(weapon2)		## does ranged weapon already exist?
	if weapon2 != 0 and weaponx2 == OBJ_HANDLE_NULL:	## if ranged weapon doesn't exist then...
    		create_item_in_inventory( weapon2, npc )	## create ranged weapon
	shieldx = npc.item_find_by_proto(shield)		## does shield already exist?
	if shield != 0 and shieldx == OBJ_HANDLE_NULL:		## if shield doesn't exist then...
		create_item_in_inventory( shield, npc )		## create shield
	npc.item_wield_best_all()				## equip everything
	npc.obj_set_int( obj_f_pad_i_9, 0 )			## reset to 0
	npc.obj_set_int( obj_f_pad_i_7, 0 )			## reset to 0
	npc.obj_set_int( obj_f_pad_i_8, 0 )			## reset to 0
	return 


##########################################################################################
## This switches to melee weapon and shield						##	
##########################################################################################
	
def get_melee_weapon(npc):
	if critter_is_unconscious(npc) == 1:  	## no reason to run script if unconcious
		return
	weapon1 = npc.obj_get_int( obj_f_pad_i_9 )		## recall melee weapon
	if weapon1 == 0:					## is this an actual number?
		return						## if no, then end routine
	item = npc.item_worn_at(3)				## find ranged weapon being used
	if item != OBJ_HANDLE_NULL:				## was ranged weapon found?
		item.destroy()					## if yes, then destroy ranged weapon
	weaponx1 = npc.item_find_by_proto(weapon1)		## does melee weapon already exist?
	if weapon1 != 0 and weaponx1 == OBJ_HANDLE_NULL:	## if it doesn't exist then...
		create_item_in_inventory( weapon1, npc )	## create melee weapon
	shield = npc.obj_get_int( obj_f_pad_i_8 )		## recall shield
	shieldx = npc.item_find_by_proto(shield)		## does shield already exist?
	if shield != 0 and shieldx == OBJ_HANDLE_NULL:		## if shield doesn't exist then...
		create_item_in_inventory( shield, npc )		## create shield
	npc.item_wield_best_all()				## equip everything
	return 


##########################################################################################
## This switches to ranged weapon only							##	
##########################################################################################
	
def get_ranged_weapon(npc):
	if critter_is_unconscious(npc) == 1:  	## no reason to run script if unconcious
		return
	weapon2 = npc.obj_get_int( obj_f_pad_i_7 )		## recall ranged weapon
	if weapon2 == 0:					## is this an actual number?
		return						## if no, then end routine
	item = npc.item_worn_at(3)				## find melee weapon being used
	if item != OBJ_HANDLE_NULL:				## was melee weapon found?
		item.destroy()					## if yes, then destroy melee weapon
	item2 = npc.item_worn_at(11)				## find shield being used
	if item2 != OBJ_HANDLE_NULL:				## was shield found?
		item2.destroy()					## if yes, then destroy shield
		## NOTE: it is important to destroy shield.
		## A one-handed ranged weapon will not equip if a shield is present.
		## Weird, but true, at least in all of my tests.
	weaponx2 = npc.item_find_by_proto(weapon2)		## does ranged weapon already exist?
	if weapon2 != 0 and weaponx2 == OBJ_HANDLE_NULL:	## if it doesn't exist then...
    		create_item_in_inventory( weapon2, npc )	## create ranged weapon
	npc.item_wield_best_all()				## equip everything
	return


##########################################################################################
## This checks for any ammo. Not specific to ranged weapon.				##	
##########################################################################################
	
def detect_ammo(npc):
	itemcount = 5004
	while (itemcount <= 5099):				## creates loop
		if ( npc.item_find( itemcount ) != OBJ_HANDLE_NULL ):
			return 1				## return 1 if ammo exists
		itemcount = itemcount + 1
	return 0						## return 0 if no ammo


##########################################################################################
## This determines if weapon equiped is ranged or melee					##	
##########################################################################################
	
def detect_weapon_type(npc):
	weapon1 = npc.obj_get_int( obj_f_pad_i_9 )		## recall melee weapon
	weapon2 = npc.obj_get_int( obj_f_pad_i_7 )		## recall ranged weapon
	weaponx = npc.item_worn_at(3)				## find weapon being used
	if weaponx.name == weapon1 and weaponx != OBJ_HANDLE_NULL:	## was melee weapon found?
		return 1					## return melee weapon equiped (1)
	if weaponx.name == weapon2 and weaponx != OBJ_HANDLE_NULL:	## was ranged weapon found?
		return 2					## return ranged weapon equiped (2)
	return 0						## return no weapon equiped (0)


##########################################################################################
## This will try to pick a strategy for a melee character (no reach weapon)		##
##											##
## It takes into account Feats, Health, and other limiting factors			##
##											##
## Useful for any Humanoid melee NPC type						##	
##########################################################################################


##### NOTE:  All float messages need to be removed before final release!!!!

def get_melee_strategy(npc):

	if critter_is_unconscious(npc) == 1:  	## no reason to run script if unconcious
		return
	
	enemyclose = 0
	enemymedium = 0
	enemyfar = 0
	enemyveryfar = 0
	prone = 0
	dying = 0
	helpless = 0
	friendclose = 0
	friendmedium = 0
	casterclose = 0

	webbed = break_free(npc, 3)  ## runs break_free script with range set to 3
			
	for obj in game.party[0].group_list():	## find number of enemy and distances
		# 5' distance is evidently equivalent to obj.distance_to(npc) == 3, and 10' to 8
		if (obj.distance_to(npc) < 3 and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Prone)) and (not obj.d20_query(Q_CoupDeGrace))):
			enemyclose = enemyclose + 1

		if (obj.distance_to(npc) < 3 and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 10 and (not obj.d20_query(Q_Prone)) and (not obj.d20_query(Q_CoupDeGrace)) and (obj.stat_level_get(stat_level_sorcerer) >= 2 or obj.stat_level_get(stat_level_wizard) >= 2) ):
			casterclose = casterclose + 1

		if (obj.distance_to(npc) < 8 and obj.distance_to(npc) >= 3 and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Prone)) and (not obj.d20_query(Q_CoupDeGrace))):
			enemymedium = enemymedium + 1

		if (obj.distance_to(npc) <= 20 and obj.distance_to(npc) >= 8 and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Prone)) and (not obj.d20_query(Q_CoupDeGrace))):
			enemyfar = enemyfar + 1

		if (obj.distance_to(npc) <= 30 and obj.distance_to(npc) >= 21 and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Prone)) and (not obj.d20_query(Q_CoupDeGrace))):
			enemyveryfar = enemyveryfar + 1

		if (obj.distance_to(npc) <= 10 and obj.d20_query(Q_Prone) and obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Helpless))):
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				obj.float_mesfile_line( 'mes\\skill_ui.mes', 106 )
			prone = prone + 1

		if (obj.distance_to(npc) <= 5 and obj.d20_query(Q_Helpless) and obj.stat_level_get( stat_hp_current ) >= 0):
			helpless = helpless + 1
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				obj.float_mesfile_line( 'mes\\skill_ui.mes', 107 )

		if (obj.distance_to(npc) <= 5 and obj.stat_level_get( stat_hp_current ) <= -1 and (not obj.d20_query(Q_Dead))):
			dying = dying + 1
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				obj.float_mesfile_line( 'mes\\skill_ui.mes', 108 )


	for obj in game.obj_list_vicinity(npc.location,OLC_NPC):
		if (obj.distance_to(npc) <= 8 and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Prone)) and (not obj.d20_query(Q_CoupDeGrace)) and obj.leader_get() == OBJ_HANDLE_NULL and obj != npc):
			friendclose = friendclose + 1
		if (obj.distance_to(npc) >= 9 and obj.distance_to(npc) <= 16 and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Prone)) and (not obj.d20_query(Q_CoupDeGrace)) and obj.leader_get() == OBJ_HANDLE_NULL and obj != npc):
			friendmedium = friendmedium + 1

	hpnownpc = npc.stat_level_get( stat_hp_current )
	hpmaxnpc = npc.stat_level_get( stat_hp_max )
	dumbass = npc.stat_level_get(stat_intelligence) < 8

	if (hpnownpc < (hpmaxnpc / 2) and (npc.item_find(8006) or npc.item_find(8007) or npc.item_find(8014) or npc.item_find(8037)) ): 
		npc.obj_set_int( obj_f_critter_strategy, 2)	## Only Use Potion w/ 5 Foot Step (2)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 602 )	
		return

	if (webbed == 1 and enemyclose == 0): 
		npc.obj_set_int( obj_f_critter_strategy, 0)	## Default (allows for break free)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 600 )	
		return

	if (npc.has_feat(feat_combat_expertise) and hpnownpc <= (hpmaxnpc / 2) and casterclose == 0 and enemyclose >= 1 and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 38)	## Combat Expertise 2 (38)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 638 )	
		return

	if (npc.has_feat(feat_combat_expertise) and enemyclose >= 2):
		npc.obj_set_int( obj_f_critter_strategy, 35)	## Combat Expertise (35)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 635 )	
		return

	if (hpnownpc <= (hpmaxnpc / 3) and casterclose == 0 and enemyclose >= 2):
		npc.obj_set_int( obj_f_critter_strategy, 37)	## Evasive maneuver 2 (37)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 637)
		return

	if (hpnownpc <= (hpmaxnpc / 2) and casterclose == 0 and enemyclose >= 1 and (friendclose >= 1 or friendmedium >= 2)):
		num = game.random_range(1,4)
		if num == 1:
			npc.obj_set_int( obj_f_critter_strategy, 36)	## Evasive maneuver (36)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 636)
			return
		else:
			npc.obj_set_int( obj_f_critter_strategy, 37)	## Evasive maneuver 2 (37)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 637)	
			return

	if (npc.has_feat(feat_barbarian_rage) and npc.has_feat(feat_power_attack) and ((casterclose == 1 and enemyclose == 1) or (casterclose == 2 and enemyclose == 2)) and hpnownpc >= (2*(hpmaxnpc / 3))):
		npc.obj_set_int( obj_f_critter_strategy, 47)	## Rage - power attack (47)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 647 )	
		return

	if (npc.has_feat(feat_power_attack) and ((casterclose == 1 and enemyclose == 1) or (casterclose == 2 and enemyclose == 2)) and hpnownpc >= (2*(hpmaxnpc / 3))):
		npc.obj_set_int( obj_f_critter_strategy, 20)	## Melee - power attack (20)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 620 )	
		return

	if (((casterclose == 1 and enemyclose == 1) or (casterclose == 2 and enemyclose == 2)) and hpnownpc >= (2*(hpmaxnpc / 3)) and (friendclose >= 1 or friendmedium >= 2)):
		npc.obj_set_int( obj_f_critter_strategy, 36)	## Melee - ready vs. spell (36)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 636 )	
		return

	if (npc.has_feat(feat_power_attack) and enemyclose == 0 and enemymedium == 0 and prone >= 1 and hpnownpc >= (2*(hpmaxnpc / 3))): 
		npc.obj_set_int( obj_f_critter_strategy, 26)	## Melee - attack prone w/ power attack (26)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 626 )	
		return

	if (npc.has_feat(feat_power_attack) and enemyclose == 0 and enemymedium == 0 and helpless >= 1 and hpnownpc >= (2*(hpmaxnpc / 3))): 
		npc.obj_set_int( obj_f_critter_strategy, 24)	## Melee - attack helpless w/ power attack (24)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 624 )	
		return

	if (enemyclose == 0 and enemymedium == 0 and prone >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 23)	## Melee - attack prone (23)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 623 )	
		return

	if (enemyclose == 0 and enemymedium == 0 and helpless >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 21)	## Melee - attack helpless (21)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 621 )	
		return

	if (npc.has_feat(feat_barbarian_rage) and npc.has_feat(feat_improved_trip) and enemyclose == 1 and hpnownpc > (hpmaxnpc / 3)):
		npc.obj_set_int( obj_f_critter_strategy, 44)	## Rage - tripper close (44)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 644 )	
		return

	if (npc.has_feat(feat_improved_trip) and enemyclose == 1 and hpnownpc > (hpmaxnpc / 3)):
		npc.obj_set_int( obj_f_critter_strategy, 27)	## Tripper - close (27)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 627 )	
		return

	if (npc.has_feat(feat_barbarian_rage) and npc.has_feat(feat_improved_trip) and enemyclose >= 1 and hpnownpc > (hpmaxnpc / 2) and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 44)	## Rage - tripper close (44)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 644 )	
		return

	if (npc.has_feat(feat_improved_trip) and enemyclose >= 1 and hpnownpc > (hpmaxnpc / 2) and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 27)	## Tripper - close (27)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 627 )	
		return

	if (npc.has_feat(feat_improved_trip) and enemyclose == 0 and hpnownpc > (2*(hpmaxnpc / 3)) and enemymedium >= 1 and (friendclose >= 1 or friendmedium >= 1)):
		num = game.random_range(1,3)
		if num == 1:
			npc.obj_set_int( obj_f_critter_strategy, 28)	## Tripper - medium (28)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 628 )
			return
		elif num == 2:
			npc.obj_set_int( obj_f_critter_strategy, 29)	## Tripper - medium (29)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 629 )
			return
		else:
			npc.obj_set_int( obj_f_critter_strategy, 30)	## Tripper - medium (30)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 630 )
			return

	if (npc.has_feat(feat_barbarian_rage) and npc.has_feat(feat_mobility) and enemyclose == 1 and hpnownpc > (hpmaxnpc / 3) and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 42)	## Rage - flanker - close w/move (42)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 642 )	
		return

	if (npc.has_feat(feat_mobility) and enemyclose == 1 and hpnownpc > (hpmaxnpc / 3) and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 16)	## Melee - Flanker - close w/move (16)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 616 )	
		return

	if (npc.has_feat(feat_barbarian_rage) and npc.has_feat(feat_mobility) and enemyclose >= 1 and hpnownpc > (hpmaxnpc / 2) and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 41)	## Rage - flanker - close (41)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 641 )	
		return

	if (npc.has_feat(feat_mobility) and enemyclose >= 1 and hpnownpc > (hpmaxnpc / 2) and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 14)	## Melee - Flanker - close (14)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 614 )	
		return

	if (npc.has_feat(feat_mobility) and enemymedium >= 1 and enemyclose == 0 and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 15)	## Melee - Flanker - medium (15)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 615 )	
		return

	if (npc.has_feat(feat_mobility) and enemyfar >= 1 and (friendclose >= 1 or friendmedium >= 1)):
		num = game.random_range(1,2)
		if num == 1:
			npc.obj_set_int( obj_f_critter_strategy, 15)	## Melee - Flanker - medium (15)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 615 )
			return
		else:
			npc.obj_set_int( obj_f_critter_strategy, 22)	## Melee - charge attack (22)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 622 )
			return

	if (npc.has_feat(feat_mobility) and npc.has_feat(feat_barbarian_rage) and enemyclose == 0 and enemymedium >= 1 and hpnownpc == hpmaxnpc and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 42)	## Rage - flanker - close w/move (42)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 642 )	
		return

	if (npc.has_feat(feat_mobility) and enemyclose == 0 and enemymedium >= 1 and hpnownpc == hpmaxnpc and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 16)	## Melee - Flanker - close w/move (16)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 616 )	
		return
	
	if (npc.has_feat(feat_power_attack) and enemyclose == 0 and enemymedium == 0 and dying >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 17)	## Coup De Grace - w/ power attack (17)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 617 )	
		return		

	if (enemyclose == 0 and enemymedium == 0 and dying >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 7)	## Only Coup De Grace - (7)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 607 )	
		return

	if (npc.has_feat(feat_barbarian_rage) and npc.has_feat(feat_power_attack) and enemyclose == 0 and hpnownpc == hpmaxnpc and enemymedium == 0 and enemyfar >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 45)	## Rage - charge w/power attack (45)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 645 )	
		return

	if (npc.has_feat(feat_power_attack) and enemyclose == 0 and hpnownpc == hpmaxnpc and enemymedium == 0 and enemyfar >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 25)	## Melee - charge attack w/ power attack (25)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 625 )	
		return

	if (npc.has_feat(feat_barbarian_rage) and enemyclose == 0 and enemymedium == 0 and enemyfar >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 46)	## Rage - charge attack (46)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 646 )	
		return

	if (enemyclose == 0 and enemymedium == 0 and (enemyfar >= 1 or enemyveryfar >= 1)): 
		npc.obj_set_int( obj_f_critter_strategy, 22)	## Melee - charge attack (22)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 622 )	
		return

	### can't find a good strategy??
	### randomly go to either original strategy or the default strategy
	num = game.random_range(1,2)
	if num == 1:
		npc.obj_set_int( obj_f_critter_strategy, 0)	## Default
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 600 )
		return
	else:
		xyx = npc.obj_get_int( obj_f_pad_i_0 )			## get original strategy
		npc.obj_set_int( obj_f_critter_strategy, xyx )		## reset strategy to original
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 599 )	## Revert Strategy

	return


##########################################################################################
## This will try to pick a strategy for a melee character with a reach weapon		##
##											##
## It takes into account Feats, Health, and other limiting factors			##
##											##
## Useful for any Humanoid melee NPC type that uses a reach weapon			##	
##########################################################################################

##### NOTE:  All float messages need to be removed before final release!!!!

def get_melee_reach_strategy(npc):
	return
	if critter_is_unconscious(npc) == 1:  	## no reason to run script if unconcious
		return
	
	enemyadjacent = 0
	enemyclose = 0
	enemymedium = 0
	enemyfar = 0
	prone = 0
	dying = 0
	helpless = 0
	friendclose = 0
	friendmedium = 0
	casterclose = 0

	webbed = break_free(npc, 8)  ## runs break_free script with range set to 8
			
	for obj in game.party[0].group_list():	## find number of enemy and distances
		# Note: 10' reach is equivalent to npc.distance_to(obj) == 8
		if (obj.distance_to(npc) < 3 and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Prone)) and (not obj.d20_query(Q_CoupDeGrace))):
			enemyadjacent = enemyadjacent + 1

		if (obj.distance_to(npc) < 8 and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Prone)) and (not obj.d20_query(Q_CoupDeGrace))):
			enemyclose = enemyclose + 1

		if (obj.distance_to(npc) < 8 and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 10 and (not obj.d20_query(Q_Prone)) and (not obj.d20_query(Q_CoupDeGrace)) and (obj.stat_level_get(stat_level_sorcerer) >= 2 or obj.stat_level_get(stat_level_wizard) >= 2) ):
			casterclose = casterclose + 1

		if (obj.distance_to(npc) < 13 and obj.distance_to(npc) >= 8 and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Prone)) and (not obj.d20_query(Q_CoupDeGrace))):
			enemymedium = enemymedium + 1

		if (obj.distance_to(npc) <= 25 and obj.distance_to(npc) > 13 and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Prone)) and (not obj.d20_query(Q_CoupDeGrace))):
			enemyfar = enemyfar + 1

		if (obj.distance_to(npc) <= 10 and obj.d20_query(Q_Prone) and obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Helpless))):
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				obj.float_mesfile_line( 'mes\\skill_ui.mes', 106 )
			prone = prone + 1

		if (obj.distance_to(npc) <= 5 and obj.d20_query(Q_Helpless) and obj.stat_level_get( stat_hp_current ) >= 0):
			helpless = helpless + 1
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				obj.float_mesfile_line( 'mes\\skill_ui.mes', 107 )

		if (obj.distance_to(npc) <= 5 and obj.stat_level_get( stat_hp_current ) <= -1 and (not obj.d20_query(Q_Dead))):
			dying = dying + 1
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				obj.float_mesfile_line( 'mes\\skill_ui.mes', 108 )


	for obj in game.obj_list_vicinity(npc.location,OLC_NPC):
		if (obj.distance_to(npc) <= 8 and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Prone)) and (not obj.d20_query(Q_CoupDeGrace)) and obj.leader_get() == OBJ_HANDLE_NULL and obj != npc):
			friendclose = friendclose + 1
		if (obj.distance_to(npc) > 8 and obj.distance_to(npc) <= 16 and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Prone)) and (not obj.d20_query(Q_CoupDeGrace)) and obj.leader_get() == OBJ_HANDLE_NULL and obj != npc):
			friendmedium = friendmedium + 1


	hpnownpc = npc.stat_level_get( stat_hp_current )
	hpmaxnpc = npc.stat_level_get( stat_hp_max )
	dumbass = npc.stat_level_get(stat_intelligence) < 8

	if (hpnownpc < (hpmaxnpc / 2) and (npc.item_find(8006) or npc.item_find(8007) or npc.item_find(8014) or npc.item_find(8037)) ): 
		npc.obj_set_int( obj_f_critter_strategy, 2)	## Only Use Potion w/ 5 Foot Step (2)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 602 )	
		return

	if (webbed == 1 and enemyclose == 0): 
		npc.obj_set_int( obj_f_critter_strategy, 0)	## Default (allows for break free)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 600 )	
		return

	if (npc.has_feat(feat_combat_expertise) and hpnownpc <= (hpmaxnpc / 2) and casterclose == 0 and enemyclose >= 1 and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 38)	## Combat Expertise 2 (38)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 638 )	
		return

	if (npc.has_feat(feat_combat_expertise) and enemyclose >= 2):
		npc.obj_set_int( obj_f_critter_strategy, 38)	## Combat Expertise 2 (38)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 638 )	
		return

	if (npc.has_feat(feat_barbarian_rage) and npc.has_feat(feat_power_attack) and ((casterclose == 1 and enemyclose == 1) or (casterclose == 2 and enemyclose == 2)) and hpnownpc >= (2*(hpmaxnpc / 3))):
		npc.obj_set_int( obj_f_critter_strategy, 47)	## Rage - power attack (47)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 647 )	
		return

	if (npc.has_feat(feat_power_attack) and ((casterclose == 1 and enemyclose == 1) or (casterclose == 2 and enemyclose == 2)) and hpnownpc >= (2*(hpmaxnpc / 3))):
		npc.obj_set_int( obj_f_critter_strategy, 20)	## Melee - power attack (20)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 620 )	
		return

	if (not dumbass and ( (casterclose == 1 and enemyclose == 1) or (casterclose == 2 and enemyclose == 2) ) and hpnownpc >= (2*(hpmaxnpc / 3)) and (friendclose >= 1 or friendmedium >= 2) ):
		npc.obj_set_int( obj_f_critter_strategy, 36)	## Melee - ready vs. spell (36)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 636 )	
		return

	if (npc.has_feat(feat_power_attack) and enemyclose == 0 and enemymedium == 0 and prone >= 1 and hpnownpc >= (2*(hpmaxnpc / 3))): 
		npc.obj_set_int( obj_f_critter_strategy, 26)	## Melee - attack prone w/ power attack (26)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 626 )	
		return

	if (npc.has_feat(feat_power_attack) and enemyclose == 0 and enemymedium == 0 and helpless >= 1 and hpnownpc >= (2*(hpmaxnpc / 3))): 
		npc.obj_set_int( obj_f_critter_strategy, 24)	## Melee - attack helpless w/ power attack (24)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 624 )	
		return

	if (enemyclose == 0 and enemymedium == 0 and prone >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 23)	## Melee - attack prone (23)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 623 )	
		return

	if (enemyclose == 0 and enemymedium == 0 and helpless >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 21)	## Melee - attack helpless (21)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 621 )	
		return

	if (npc.has_feat(feat_barbarian_rage) and npc.has_feat(feat_improved_trip) and enemyclose == 1 and hpnownpc > (hpmaxnpc / 3)):
		npc.obj_set_int( obj_f_critter_strategy, 44)	## Rage - tripper close (44)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 644 )	
		return

	if (npc.has_feat(feat_improved_trip) and enemyclose == 1 and hpnownpc > (hpmaxnpc / 3)):
		npc.obj_set_int( obj_f_critter_strategy, 27)	## Tripper - close (27)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 627 )	
		return

	if (npc.has_feat(feat_barbarian_rage) and npc.has_feat(feat_improved_trip) and enemyclose >= 1 and hpnownpc > (hpmaxnpc / 2) and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 44)	## Rage - tripper close (44)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 644 )	
		return

	if (npc.has_feat(feat_improved_trip) and enemyclose >= 1 and hpnownpc > (hpmaxnpc / 2) and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 27)	## Tripper - close (27)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 627 )	
		return

	if (npc.has_feat(feat_improved_trip) and enemyclose == 0 and hpnownpc > (2*(hpmaxnpc / 3)) and enemymedium >= 1 and (friendclose >= 1 or friendmedium >= 1)):
		num = game.random_range(1,3)
		if num == 1:
			npc.obj_set_int( obj_f_critter_strategy, 28)	## Tripper - medium (28)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 628 )
			return
		elif num == 2:
			npc.obj_set_int( obj_f_critter_strategy, 29)	## Tripper - medium (29)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 629 )
			return
		else:
			npc.obj_set_int( obj_f_critter_strategy, 30)	## Tripper - medium (30)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 630 )
			return

	if (npc.has_feat(feat_barbarian_rage) and npc.has_feat(feat_mobility) and enemyclose == 1 and hpnownpc > (hpmaxnpc / 3) and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 42)	## Rage - flanker - close w/move (42)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 642 )	
		return

	if (npc.has_feat(feat_mobility) and enemyclose == 1 and hpnownpc > (hpmaxnpc / 3) and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 16)	## Melee - Flanker - close w/move (16)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 616 )	
		return

	if (npc.has_feat(feat_barbarian_rage) and npc.has_feat(feat_mobility) and enemyclose >= 1 and hpnownpc > (hpmaxnpc / 2) and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 41)	## Rage - flanker - close (41)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 641 )	
		return

	if (npc.has_feat(feat_mobility) and enemyclose >= 1 and hpnownpc > (hpmaxnpc / 2) and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 14)	## Melee - Flanker - close (14)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 614 )	
		return

	if (npc.has_feat(feat_mobility) and enemymedium >= 1 and enemyclose == 0 and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 15)	## Melee - Flanker - medium (15)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 615 )	
		return

	if (npc.has_feat(feat_mobility) and enemyfar >= 1 and (friendclose >= 1 or friendmedium >= 1)):
		num = game.random_range(1,2)
		if num == 1:
			npc.obj_set_int( obj_f_critter_strategy, 15)	## Melee - Flanker - medium (15)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 615 )
			return
		else:
			npc.obj_set_int( obj_f_critter_strategy, 22)	## Melee - charge attack (22)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 622 )
			return

	if (npc.has_feat(feat_mobility) and npc.has_feat(feat_barbarian_rage) and enemyclose == 0 and enemymedium >= 1 and hpnownpc == hpmaxnpc and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 42)	## Rage - flanker - close w/move (42)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 642 )	
		return

	if (npc.has_feat(feat_mobility) and enemyclose == 0 and enemymedium >= 1 and hpnownpc == hpmaxnpc and (friendclose >= 1 or friendmedium >= 1)):
		npc.obj_set_int( obj_f_critter_strategy, 16)	## Melee - Flanker - close w/move (16)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 616 )	
		return
	
	if (npc.has_feat(feat_power_attack) and enemyclose == 0 and enemymedium == 0 and dying >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 17)	## Coup De Grace - w/ power attack (17)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 617 )	
		return		

	if (enemyclose == 0 and enemymedium == 0 and dying >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 7)	## Only Coup De Grace - (7)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 607 )	
		return

	if (npc.has_feat(feat_barbarian_rage) and npc.has_feat(feat_power_attack) and enemyclose == 0 and hpnownpc == hpmaxnpc and enemymedium == 0 and enemyfar >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 45)	## Rage - charge w/power attack (45)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 645 )	
		return

	if (npc.has_feat(feat_power_attack) and enemyclose == 0 and hpnownpc == hpmaxnpc and enemymedium == 0 and enemyfar >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 25)	## Melee - charge attack w/ power attack (25)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 625 )	
		return

	if (npc.has_feat(feat_barbarian_rage) and enemyclose == 0 and enemymedium == 0 and enemyfar >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 46)	## Rage - charge attack (46)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 646 )	
		return

	if (enemyclose == 0 and enemymedium == 0 and enemyfar >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 22)	## Melee - charge attack (22)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 622 )	
		return

	if (npc.has_feat(feat_barbarian_rage) and enemyclose >= 1): 
		npc.obj_set_int( obj_f_critter_strategy, 43)	## Rage - reach weapon close (43)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 643 )	
		return

	if (enemyadjacent >= 1): # attack victim and back off with a 5' step at the end
		npc.obj_set_int( obj_f_critter_strategy, 19)	## Melee - Reach Weapon Close (19)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 619 )	
		return

	if (enemyadjacent == 0 and enemyclose >= 1): # stand in place and whack away
		npc.obj_set_int( obj_f_critter_strategy, 7)	## Only Coup De Grace - (7) (even though no coup de grace is there??? -SA)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 607 )	
		return

	### can't find a good strategy??
	### randomly go to either original strategy or the default strategy
	num = game.random_range(1,2)
	if num == 1:
		npc.obj_set_int( obj_f_critter_strategy, 18)	## Melee - Reach Weapon Default (18)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 618 )
		return
	else:
		xyx = npc.obj_get_int( obj_f_pad_i_0 )			## get original strategy
		npc.obj_set_int( obj_f_critter_strategy, xyx )		## reset strategy to original
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 599 )	## Revert Strategy

	return

##########################################################################################
## This will try to pick a strategy for a ranged NPC					##	
##########################################################################################

##### NOTE:  All float messages need to be removed before final release!!!!
	
def get_ranged_strategy(npc):

	if critter_is_unconscious(npc) == 1:  	## no reason to run script if unconcious
		return

	caster = 0
	frenz = 0

	for co_com in game.obj_list_vicinity(npc.location,OLC_NPC):	## find friendlies
		if (co_com.is_friendly( npc ) and critter_is_unconscious(co_com) == 0):
			frenz = frenz + 1

	for obj in game.party[0].group_list():	## find casters
		if (obj.stat_level_get( stat_hp_current ) >= 1 and (not obj.d20_query(Q_Helpless)) and (not obj.d20_query(Q_CoupDeGrace)) and (obj.stat_level_get(stat_level_sorcerer) >= 1 or obj.stat_level_get(stat_level_wizard) >= 1 or obj.stat_level_get(stat_level_bard) >= 2 or obj.stat_level_get(stat_level_cleric) >= 2 or obj.stat_level_get(stat_level_druid) >= 1) ):
			caster = caster + 1


	if (caster >= 3 and frenz > 4): 
		num = game.random_range(1,14)
		if num == 1:
			npc.obj_set_int( obj_f_critter_strategy, 10)	## Ranged - Sniper 1 (10)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 610 )
			return
		elif num == 2:
			npc.obj_set_int( obj_f_critter_strategy, 11)	## Ranged - Sniper 2 (11)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 611 )
			return
		elif num == 3:
			npc.obj_set_int( obj_f_critter_strategy, 12)	## Ranged - Sniper 3 (12)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 612 )
			return
		elif num == 4:
			npc.obj_set_int( obj_f_critter_strategy, 8)	## Ranged - Sniper (8)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 608 )
			return
		else:
			npc.obj_set_int( obj_f_critter_strategy, 9)	## Ranged - Ready vs. Spell (9)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 609 )
			return

	if ((caster >= 2 and frenz > 1) or (frenz > 5 and caster > 0)):
		num = game.random_range(1,5)
		if num == 1:
			npc.obj_set_int( obj_f_critter_strategy, 10)	## Ranged - Sniper 1 (10)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 610 )
			return
		elif num == 2:
			npc.obj_set_int( obj_f_critter_strategy, 11)	## Ranged - Sniper 2 (11)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 611 )
			return
		elif num == 3:
			npc.obj_set_int( obj_f_critter_strategy, 12)	## Ranged - Sniper 3 (12)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 612 )
			return
		elif num == 4:
			npc.obj_set_int( obj_f_critter_strategy, 8)	## Ranged - Sniper (8)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 608 )
			return
		else:
			npc.obj_set_int( obj_f_critter_strategy, 9)	## Ranged - Ready vs. Spell (9)
			if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
				npc.float_mesfile_line( 'mes\\skill_ui.mes', 609 )
			return	

	num = game.random_range(1,5)
	if num == 1:
		npc.obj_set_int( obj_f_critter_strategy, 10)	## Ranged - Sniper 1 (10)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 610 )
		return
	elif num == 2:
		npc.obj_set_int( obj_f_critter_strategy, 11)	## Ranged - Sniper 2 (11)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 611 )
		return
	elif num == 3:
		npc.obj_set_int( obj_f_critter_strategy, 12)	## Ranged - Sniper 3 (12)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 612 )
		return
	elif num == 4:
		npc.obj_set_int( obj_f_critter_strategy, 8)	## Ranged - Sniper (8)
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 608 )
		return
	else:
		xyx = npc.obj_get_int( obj_f_pad_i_0 )			## get original strategy
		npc.obj_set_int( obj_f_critter_strategy, xyx )		## reset strategy to original
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 599 )	## Revert Strategy
	
	return


##########################################################################################
## This records the original strategy from the protos.tab				##	
##########################################################################################
	
def tag_strategy(npc):					
	xyx = npc.obj_get_int( obj_f_critter_strategy )		## get original strategy
	npc.obj_set_int( obj_f_pad_i_0, xyx )			## record the original strategy
	return


##########################################################################################
## This picks a strategy at random from 5 options					##	
##########################################################################################

def change_strategy_random(npc, x1, x2, x3, x4, x5):		## 5 possible scripts
	num = game.random_range(1,5)				## random number picked
	if num == 1:
		npc.obj_set_int( obj_f_critter_strategy, x1 )	## set new strategy
	if num == 2:
		npc.obj_set_int( obj_f_critter_strategy, x2 )	## set new strategy
	if num == 3:
		npc.obj_set_int( obj_f_critter_strategy, x3 )	## set new strategy
	if num == 4:
		npc.obj_set_int( obj_f_critter_strategy, x4 )	## set new strategy
	else:
		npc.obj_set_int( obj_f_critter_strategy, x5 )	## set new strategy
	return


##########################################################################################
## This changes to a specific script							##	
##########################################################################################
	
def change_strategy(npc, x1):					## x1 is new script number
	npc.obj_set_int( obj_f_critter_strategy, x1 )		## set new strategy
	return


##########################################################################################
## This reverts to the original AI NPC script						##	
##########################################################################################

def revert_strategy(npc):
	xyx = npc.obj_get_int( obj_f_pad_i_0 )			### get original strategy
	npc.obj_set_int( obj_f_critter_strategy, xyx )		### reset strategy to original
	return


##########################################################################################
## Checks current strategy against original strategy					##	
##########################################################################################

def same_strategy(npc):					
	ors = npc.obj_get_int( obj_f_critter_strategy )		### get current strategy
	nes = npc.obj_get_int( obj_f_pad_i_0 )			### get original strategy
	if ors == nes:						### are they the same?
		return 1					### return 1 if same
	return 0						### return 0 if different


##########################################################################################
## Detects if a talker is present							##	
##########################################################################################

def detect_talker(talker, seeker):
	if (find_npc_near(talker,seeker) != OBJ_HANDLE_NULL):
		return 1
	return 0


##########################################################################################
## Changes to new talker								##	
##########################################################################################

def change_talker( talker, seeker, pc, line ):
	target = find_npc_near(talker,seeker)
	pc.begin_dialog(target,line)
	target.turn_towards(pc)
	pc.turn_towards(target)
	return SKIP_DEFAULT


##########################################################################################
## This calls for the Break Free from Web routine					##	
##########################################################################################

def break_free(npc, range = '5ft'):
	if range == '10ft':
		range = 8
	elif range == '5ft':
		range = 3
	for obj in game.party[0].group_list():
		if (obj.distance_to(npc) <= range and obj.stat_level_get(stat_hp_current) >= -9):
			return RUN_DEFAULT
	while(npc.item_find(8903) != OBJ_HANDLE_NULL):
		npc.item_find(8903).destroy()
	if (npc.d20_query(Q_Is_BreakFree_Possible)):
		create_item_in_inventory( 8903, npc )
		return 1
	return 0



##########################################################################################
## This tags and destroys the NPC's thrown weapon, melee weapon (no shield please)	##	
## As a fail safe, if you run this script with an NPC that has a shield			##	
## The shield will vanish								##
## obj_f_pad_i_7 records the type of thrown weapon					##
## obj_f_pad_i_9 records the type of melee weapon					##
## obj_f_pad_i_8 records quantity of thrown weapon					##
##########################################################################################

def tag_weapons_thrown(npc):
	npc.item_wield_best_all()				## equip something
	shieldx = npc.item_worn_at(11)				## find shield being used
	if shieldx != OBJ_HANDLE_NULL:				## was a shield found?
		shieldx.destroy()				## destroy shield

	npc.item_wield_best_all()				## equip something
	weaponx = npc.item_worn_at(3)				## find weapon being used
	rnum = 0						## reset counter
	while(weaponx != OBJ_HANDLE_NULL):			## start loop
		npc.item_wield_best_all()			## equip something
		shieldx = npc.item_worn_at(11)			## find shield being used
		if shieldx != OBJ_HANDLE_NULL:			## was a shield found?
			shieldx.destroy()			## destroy shield
			npc.item_wield_best_all()		## equip something
		x1 = weaponx.obj_get_int( obj_f_weapon_flags )	## check weapon flag type
		if (x1 == 1024):				## if weapon is ranged
			weapon = weaponx.name			## get weapon's protos name
			npc.obj_set_int( obj_f_pad_i_7, weapon )## record thrown weapon's name
			weaponx.destroy() 			## destroy thrown weapon
			rnum = rnum + 1				## add to count
		else:						## if weapon is not ranged
			weapon = weaponx.name			## get weapon's protos name
			npc.obj_set_int( obj_f_pad_i_9, weapon )## record melee weapon's name
			weaponx.destroy()			## destoy melee weapon	
		npc.item_wield_best_all()			## equip something
		weaponx = npc.item_worn_at(3)			## find weapon being used
	npc.obj_set_int( obj_f_pad_i_8, rnum )  		## record quantity of thrown weapon
	npc.item_wield_best_all()				## equip something
	
	return


##########################################################################################
## This recalls all weapons, and resets variables to 0					##	
##########################################################################################
	
def get_everything_thrown(npc): 
	weapon1 = npc.obj_get_int( obj_f_pad_i_9 )		## recall melee weapon
	weapon2 = npc.obj_get_int( obj_f_pad_i_7 )		## recall thrown weapon
	quantity = npc.obj_get_int( obj_f_pad_i_8 )		## recall quantity of thrown weapons
	if weapon1 == 0 and weapon2 == 0 and quantity == 0:	## nothing set?
		return						## end routine
	weaponx1 = npc.item_find_by_proto(weapon1)		## does melee weapon already exist?
	if weapon1 != 0 and weaponx1 == OBJ_HANDLE_NULL:	## if melee weapons doesn't exist then...
		create_item_in_inventory( weapon1, npc )	## create melee weapon
	
	while(quantity >= 1):					## start loop	
		create_item_in_inventory( weapon2, npc )	## create thrown weapon
		quantity = quantity - 1				## reduce counter

	npc.obj_set_int( obj_f_pad_i_9, 0 )			## reset to 0
	npc.obj_set_int( obj_f_pad_i_7, 0 )			## reset to 0
	npc.obj_set_int( obj_f_pad_i_8, 0 )			## reset to 0
	return 


##########################################################################################
## This holds melee weapon only (should only follow a tag call)				##	
##########################################################################################

def hold_melee_weapon_thrown(npc):
	weapon1 = npc.obj_get_int( obj_f_pad_i_9 )		## recall melee weapon
	if weapon1 == 0:					## is this an actual number?
		return						## if no, then end routine
	weaponx1 = npc.item_find_by_proto(weapon1)		## does melee weapon already exist?
	if weapon1 != 0 and weaponx1 == OBJ_HANDLE_NULL:	## if it doesn't exist then...
		create_item_in_inventory( weapon1, npc )	## create melee weapon
	npc.item_wield_best_all()				## equip everything
	return 


##########################################################################################
## This holds ranged weapon only (should only follow a tag call)			##	
##########################################################################################

def hold_ranged_weapon_thrown(npc):
	weapon2 = npc.obj_get_int( obj_f_pad_i_7 )		## recall ranged weapon
	if weapon2 == 0:					## is this an actual number?
		return						## if no, then end routine
	quantity = npc.obj_get_int( obj_f_pad_i_8)		## recall quantity
	while(quantity >= 1):					## start loop	
		create_item_in_inventory( weapon2, npc )	## create thrown weapon
		quantity = quantity - 1				## reduce counter
	npc.obj_set_int( obj_f_pad_i_8, 0 )			## reset to 0
	npc.item_wield_best_all()				## equip everything
	return


##########################################################################################
## This switches to melee weapon and destroys thrown weapons... keeps a count		##	
##########################################################################################

def get_melee_weapon_thrown(npc):
	weapon1 = npc.obj_get_int( obj_f_pad_i_9 )		## recall melee weapon
	if weapon1 == 0:					## is this an actual number?
		return						## if no, then end routine
	npc.item_wield_best_all()				## equip something
	weaponx = npc.item_worn_at(3)				## find weapon being used
	rnum = 0						## reset counter
	while(weaponx != OBJ_HANDLE_NULL):			## start loop
		npc.item_wield_best_all()			## equip something
		shieldx = npc.item_worn_at(11)			## find shield being used
		if shieldx != OBJ_HANDLE_NULL:			## was a shield found?
			shieldx.destroy()			## destroy shield
			npc.item_wield_best_all()		## equip something
		x1 = weaponx.obj_get_int( obj_f_weapon_flags )	## check weapon flag type
		if (x1 == 1024):				## if weapon is ranged
			weaponx.destroy() 			## destroy thrown weapon
			rnum = rnum + 1				## add to count
		else:						## if weapon is not ranged
			weaponx.destroy()			## destoy melee weapon	
		npc.item_wield_best_all()			## equip something
		weaponx = npc.item_worn_at(3)			## find weapon being used
	npc.obj_set_int( obj_f_pad_i_8, rnum )  		## record quantity of thrown weapon
	weaponx1 = npc.item_find_by_proto(weapon1)		## does melee weapon already exist?
	if weaponx1 == OBJ_HANDLE_NULL:				## if it doesn't exist then...
		create_item_in_inventory( weapon1, npc )	## create melee weapon
	npc.item_wield_best_all()				## equip everything
	return 


##########################################################################################
## This switches to ranged weapon only							##	
##########################################################################################

def get_ranged_weapon_thrown(npc):
	weapon2 = npc.obj_get_int( obj_f_pad_i_7 )		## recall ranged weapon
	if weapon2 == 0:					## is this an actual number?
		return						## if no, then end routine
	item = npc.item_worn_at(3)				## find melee weapon being used
	if item != OBJ_HANDLE_NULL:				## was melee weapon found?
		item.destroy()					## if yes, then destroy melee weapon
	item2 = npc.item_worn_at(11)				## find shield being used
	if item2 != OBJ_HANDLE_NULL:				## was shield found?
		item2.destroy()					## if yes, then destroy shield
		## NOTE: it is important to destroy shield.
		## A one-handed ranged weapon will not equip if a shield is present.
		## Weird, but true, at least in all of my tests.
	quantity = npc.obj_get_int( obj_f_pad_i_8)		## recall quantity
	while(quantity >= 1):					## start loop	
		create_item_in_inventory( weapon2, npc )	## create thrown weapon
		quantity = quantity - 1				## reduce counter
	npc.obj_set_int( obj_f_pad_i_8, 0 )			## reset to 0
	npc.item_wield_best_all()				## equip everything
	return


##########################################################################################
## This checks for thrown weapons							##	
##########################################################################################
	
def detect_thrown_ammo(npc, type):
	if type == 1:
		quantity = npc.obj_get_int( obj_f_pad_i_8)		## recall quantity
		if quantity >= 1:					## all gone...
			return 1					## return 1 if ammo exists
		return 0						## return 0 if no ammo
	else:
		npc.item_wield_best_all()				## equip something
		weaponx = npc.item_worn_at(3)				## find a weapon
		if (weaponx == OBJ_HANDLE_NULL):			## no weapon found
			npc.obj_set_int( obj_f_pad_i_8, 0 )		## set quantity to 0
			return 0					## return 0 if no ammo
		return 1						## return 1 if ammo exists


##########################################################################################
## Is there a target in range?								##	
##########################################################################################
	
def target_distance(npc, distx):
	for obj in game.party[0].group_list():	
		if (obj.distance_to(npc) <= distx and critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 1):
			return 1
	return 0


##########################################################################################
## This attempts to find a suitable candidate for a area effect spell			##
## distx = the size of area effect							##
## fedead = maximum number of friends you are willing to hit				##
## edead = minimum number of enemy units you want to hit				##
##											##
## If a target is found it will record the target as obj_f_npc_pad_i_5			##
## This allows area effect spells to define this target as their target			##
##											##
## The script returns 0 if no target is found and it returns 1 if target found		##
##########################################################################################

#### NOTE: I don't know how to deal with projectiles, so for Fireball it is pointless to
#### 	to set "fedead" at anything less than 0 as I rely on the "cast fireball" call from
####	the strategy.tab to find the best target.  I hope to improve this at a later date.
####
####	Area effect spells that do work with "fedead" are:
####		Silence		(Needs to be called from strategy.tab by "cast single"
####		Web		(Needs to be called from strategy.tab by "cast single"
####
####
####	Note Silence variation allowed by adding an additional 1,000 to the obj_f_npc_pad_i_5

def target_area_spell(npc, distx, fdead, edead):
	caster = 0
	target = 666
	targetF = 0
	targetE = 0
	count = 0

	xyx = npc.obj_get_int( obj_f_npc_pad_i_5)		## clears past targetting
	if xyx >= 1000:						## clears past targetting
		npc.obj_set_int( obj_f_npc_pad_i_5, 0)		## clears past targetting

	for obj in game.party[0].group_list():	## check each party member
		friendlies = 0
		enemies = 0
		
		for obj2 in game.obj_list_vicinity(obj.location,OLC_NPC):	## searches for friendlies
			if (obj2.distance_to(obj) <= (distx/2) and obj2.is_friendly(npc)) and obj2.stat_level_get(stat_hp_current) >= 0:
				friendlies = friendlies + 1

			if (obj2.distance_to(obj) <= (distx/2) and obj2.stat_level_get(stat_hp_current) >= 0 and (not obj2.is_friendly(npc))):
				enemies = enemies + 1

		for obj2 in game.obj_list_vicinity(obj.location,OLC_PC):	## searches for friendlies
			if (obj2.distance_to(obj) <= (distx/2) and obj2.is_friendly(npc) and obj2.stat_level_get(stat_hp_current) >= 0):
				friendlies = friendlies + 1

			if (obj2.distance_to(obj) <= (distx/2) and obj2.stat_level_get(stat_hp_current) >= 0 and (not obj2.is_friendly(npc))):
				enemies = enemies + 1

		if (friendlies <= fdead and enemies >= targetE and enemies >= edead):
			target = count
			targetF = friendlies
			targetE = enemies
		count = count + 1

	if target == 666:
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 501)
		return 0
	else:
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 580+target )
		npc.obj_set_int( obj_f_npc_pad_i_5, 1000+target )
		npc.turn_towards(game.party[target])

	return 1

##########################################################################################
## Find candidate for Silence								##
## type = kind of spell caster								##
##											##
## 0 = highest any kind									##
## 1 = Sorcerer										##
## 2 = Wizard										##
## 3 = Druid										##
## 4 = Cleric										##
## 5 = Bard										##
##											##
## passes target to obj_f_npc_pad_i_5							##
## if Silence is cast then Silence will re-target based on obj_f_npc_pad_i_5		##
##########################################################################################
	
def target_for_silence(npc, type):
	target = 666
	Hlevel = 0
	count = 0

	for obj in game.party[0].group_list():
		levelS = 0
		levelW = 0
		levelC = 0
		levelB = 0
		levelD = 0
		NewHlevel = 0
		if ( critter_is_unconscious(obj) == 0 and obj.stat_level_get( stat_hp_current ) >= 0 and (obj.stat_level_get(stat_level_sorcerer) >= 1 or obj.stat_level_get(stat_level_wizard) >= 1 or obj.stat_level_get(stat_level_druid) >= 1 or obj.stat_level_get(stat_level_cleric) >= 1 or obj.stat_level_get(stat_level_bard) >= 1) ):
			levelS = obj.stat_level_get(stat_level_sorcerer)
			levelW = obj.stat_level_get(stat_level_wizard)
			levelC = obj.stat_level_get(stat_level_cleric)
			levelB = obj.stat_level_get(stat_level_bard)
			levelD = obj.stat_level_get(stat_level_druid)
			if (levelB >= levelS and levelB >= levelC and levelB >= levelW and levelB >= levelD and (type == 5 or type == 0)):
				NewHlevel = levelB
			if (levelC >= levelS and levelC >= levelB and levelC >= levelW and levelC >= levelD and (type == 4 or type == 0)):
				NewHlevel = levelC
			if (levelD >= levelS and levelD >= levelB and levelD >= levelW and levelD >= levelD and (type == 3 or type == 0)):
				NewHlevel = levelD
			if (levelW >= levelS and levelW >= levelC and levelW >= levelB and levelW >= levelD and (type == 2 or type == 0)):
				NewHlevel = levelW
			if (levelS >= levelW and levelS >= levelC and levelS >= levelB and levelS >= levelD and (type == 1 or type == 0)):
				NewHlevel = levelS

			if (NewHlevel >= Hlevel and NewHlevel != 0):
				Hlevel = NewHlevel
				target = count
		count = count + 1
	if target == 666:
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 501)
		return 0
	else:
		if game.global_flags[403] == 1 and game.global_flags[405] == 1 and game.global_flags[405] == 1:
			npc.float_mesfile_line( 'mes\\skill_ui.mes', 580+target )
		npc.obj_set_int( obj_f_npc_pad_i_5, 2000+target )
		npc.turn_towards(game.party[target])

	return 1


##########################################################################################
## This detects area effect spells							##
##											##
## Area effect spells records their spell number as obj_f_pad_i_9			##
##											##
## The script returns 0 if none present returns ## if one is present			##
##											##
## when called second entry (type) should be set to:					##
##	0 for any									##
##	spell number for specific spell							##
##########################################################################################

#### This works with the following area effect spells
#### Number is what is used/returned for each area effect
####
####	Silence	-- 434	
####	Web -- 531
####	Fog Cloud -- 183
####	Stinking Cloud -- 460
####	Grease -- 200
####	Cloudkill -- 65
####    Consecrate -- 74
####	Desecrate -- 107
####	Entangle -- 153
####
####
####

def find_area_effect_spell(npc, type):

	distance = 0
	found = 0
	spell = 0
	count = 0
	
	for obj in game.obj_list_vicinity(npc.location,OLC_GENERIC):
		NEWdistance = 666
		NEWfound = 0	
		if (obj.name == 12003 and type == 0):
			NEWdistance = npc.distance_to(obj)
			NEWfound = obj
		if (obj.name == 12003 and type != 0):
			spell = obj.obj_get_int( obj_f_pad_i_9)
			if spell == type:
				NEWdistance = npc.distance_to(obj)
				NEWfound = obj
		if found != 0 and NEWdistance <= distance:
			found = NEWfound
			distance = NEWdistance	
		if found == 0 and NEWfound != 0:
			found = NEWfound
			distance = NEWdistance	
	if found == 0:
		return 0
	else:
		spell = found.obj_get_int( obj_f_pad_i_9)

	return spell


##########################################################################################
## Determines if target is in spell radius						##
## type is spell number, and radius is the spell effect radius				##	
##########################################################################################

def am_I_in_the_effect(npc, type, radius):
	for obj in game.obj_list_vicinity(npc.location,OLC_GENERIC):
		if (obj.name == 12003 and npc.distance_to(obj) <= radius/2):
			spell = obj.obj_get_int( obj_f_pad_i_9)
			if spell == type:
				return 1  ### yes, I am in the area effect
	return 0  ## no, I am not in the area effect


##########################################################################################
## Destroy's area effect spell counters							##	
##########################################################################################

def kill_area_effect_spell(npc):
	for obj in game.obj_list_vicinity(npc.location,OLC_GENERIC):
		if (obj.name == 12003):
			obj.destroy()
	return

#### Notes:
#### Area effect counters do not disappear ever.  Apparently they stay around forever.
#### Very weird.  So if you cast spells in the same space over and over again, then
#### there will just be tons of these counters laying around.
#### So if you want to have an NPC detect an area effect then you need to make sure 
#### that the NPC destroys the old useless area effect on first hearbeat, or from
#### heartbeat, but make sure you ONLY do this while combat is not active or you will
#### destroy area effect spells that are active!!!!


	