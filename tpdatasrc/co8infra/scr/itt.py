from toee import *
from utilities import *
from math import sqrt

# obj_f_npc_pad_i_4 - stores earth critters Standpoint_ID



# Note: using globals incurs a bug - globals are retained across saves...
# i.e. I tried using global reserve_melee, it's fucked
# And python apparently doesn't have static vars


# Temple Level 1 regroup reserves
bugbears_north_of_romag_melee = [(435, 424, 14163),(430, 441,14165),(424, 435, 14165),(432, 437, 14165)]
bugbears_north_of_romag_ranged = [(423, 440, 14164),(433, 428, 14164)]
earth_com_room_bugbears = [(452, 456, 14165),(446, 456, 14165),(471, 478, 14165),(465, 478, 14165)]
turnkey_room = [(568,462,14165),(569,458,14229)]
ogre_chief_rooms_melee = [(458,532,14078),(519,533,14078)]
ogre_chief_rooms_ranged = [(470,530,14164),(520,542,1416)]




def earth_reg():

	reserve_melee = [(508,477,14162), (499 , 476 , 14165), (449 , 449 , 14163)]
	reserve_melee = reserve_melee + bugbears_north_of_romag_melee + earth_com_room_bugbears + turnkey_room + ogre_chief_rooms_melee
	reserve_ranged = bugbears_north_of_romag_ranged + ogre_chief_rooms_ranged

	reserve_melee = trim_dead(reserve_melee)
	reserve_ranged = trim_dead(reserve_ranged)

	#see rnte() for parameter explanation
	#                                    (          xxx , yyy , ID    , SP_ID,n_x , n_y , rot , dist, com,   reserve list )
	reserve_melee, earth_bugbear_1 = rnte(          523 , 414 , 14163 , 650 , 479 , 387 , 4.3  , 1, 'melee', reserve_melee)
	reserve_melee, earth_bugbear_2 = rnte(          440 , 445 , 14162 , 651 , 490 , 383 , 4.3  , 1, 'melee', reserve_melee)
	reserve_melee, earth_bugbear_3 = rnte(          505 , 473 , 14163 , 652 , 475 , 394 , 4.3  , 1, 'melee', reserve_melee)

	dummy,         earth_ogre1 = rnte(              517 , 541 , 14249 , 658 , 491 , 389 , 5.5  , 2, 'big',[])
	dummy,         earth_ogre2 = rnte(              498 , 531 , 14249 , 659 , 474 , 388 , 5.3  , 2, 'big',[])
	reserve_melee, earth_barbarian_gnoll1 = rnte(   508 , 531 , 14078 , 660 , 488 , 381 , 3.5  , 2, 'melee', reserve_melee)
	reserve_melee, earth_sentry_fore1 = rnte(       417 , 441 , 14163 , 661 , 497 , 381 , 5.4  , 2, 'melee', reserve_melee)
	reserve_melee, earth_sentry_fore2 = rnte(       441 , 449 , 14163 , 662 , 470 , 384 , 5.5  , 2, 'melee', reserve_melee)
	reserve_melee, earth_sentry_back1 = rnte(       426 , 432 , 14162 , 663 , 493 , 403 , 5.45 , 2, 'melee', reserve_melee)
	reserve_melee, earth_sentry_back2 = rnte(       519 , 416 , 14162 , 664 , 471 , 399 , 5.54 , 2, 'melee', reserve_melee)

	dummy,         earth_robe_guard1 = rnte(        473 , 404 , 14337 , 665 , 480 , 422 , 4.7  , 4, 'useless',[])
	dummy,         earth_robe_guard2 = rnte(        482 , 396 , 14337 , 666 , 493 , 423 , 6    , 4, 'useless',[])
	dummy,         earth_robe_guard3 = rnte(        489 , 402 , 14337 , 667 , 474 , 433 , 4.2  , 4, 'useless',[])

	reserve_ranged, earth_archer1 = rnte(           505 , 530 , 14164 , 669 , 473 , 405 , 5.4  , 3, 'ranged', reserve_ranged)
	reserve_ranged, earth_archer2 = rnte(           502 , 473 , 14164 , 670 , 491 , 397 , 5.4  , 3, 'ranged', reserve_ranged)
	reserve_ranged, earth_archer3 = rnte(           475 , 542 , 14164 , 671 , 493 , 432 , 5.4  , 3, 'ranged', reserve_ranged)

	dummy,         earth_troop_commander = rnte(    446 , 470 , 14156 , 683 , 497 , 386 , 5.3  , 10, 'special',[])

	dummy,         earth_elemental_medium_1 = rnte( 493 , 391 , 14381 , 684 , 495 , 412 , 5.4  , 2, 'big',[])
	dummy,         earth_elemental_medium_2 = rnte( 474 , 392 , 14381 , 685 , 470 , 394 , 4.3  , 2, 'big',[])
	if earth_elemental_medium_1 != OBJ_HANDLE_NULL:
		earth_elemental_medium_1.obj_set_int(obj_f_speed_run, 1073334444)
		earth_elemental_medium_1.obj_set_int(obj_f_speed_walk, 1073334444)
	if earth_elemental_medium_2 != OBJ_HANDLE_NULL:
		earth_elemental_medium_2.obj_set_int(obj_f_speed_run, 1073334444)
		earth_elemental_medium_2.obj_set_int(obj_f_speed_walk, 1073334444)
	dummy,         earth_elemental_large = rnte(    483 , 423 , 14296 , 686 , 492 , 393 , 5.58 , 2, 'big',[])
	if earth_elemental_large != OBJ_HANDLE_NULL:
		earth_elemental_large.obj_set_int(obj_f_speed_run, 1073334444)
		earth_elemental_large.obj_set_int(obj_f_speed_walk, 1073334444)
	#	earth_elemental_large.unconceal()  # to prevent lag

	dummy,         ogrechief = rnte(                471 , 537 , 14248 , 687 , 467 , 385 , 4.8  , 50, 'special',[])
	dummy,         romag = rnte(                    445 , 444 , 8045  , 688 , 482 , 398 , 5.5  , 50, 'special',[])
	dummy,         hartsch = rnte(                  445 , 444 , 14154 , 689 , 470 , 403 , 5    , 50, 'special',[])
	dummy,         gnoll_leader = rnte(             505 , 534 , 14066 , 690 , 496 , 402 , 0.4  , 50, 'special',[])
	dummy,         earth_lieutenant = rnte(         442 , 458 , 14339 , 691 , 470 , 390 , 5.1  , 5 , 'lieutenant',[])

	reserve_melee, earth_fighter1 = rnte(           439 , 492 , 14338 , 692 , 485 , 387 , 5.4  , 1, 'melee', reserve_melee)
	reserve_melee, earth_fighter2 = rnte(           441 , 490 , 14338 , 693 , 479 , 385 , 1    , 1, 'melee', reserve_melee)
	reserve_melee, earth_fighter3 = rnte(           444 , 494 , 14338 , 694 , 481 , 391 , 4.9  , 1, 'melee', reserve_melee)
	#if earth_fighter1 == OBJ_HANDLE_NULL:
	#	earth_bugbear_4 = rnte(  508 , 477 , 14162 , 692 , 485 , 387 , 5.4  , 1)
	#elif earth_fighter1.stat_level_get(stat_hp_current) < 0:
	#	earth_bugbear_4 = rnte(  508 , 477 , 14162 , 692 , 485 , 387 , 5.4  , 1)
	#if earth_fighter2 == OBJ_HANDLE_NULL:
	#	earth_bugbear_5 = rnte(  499 , 476 , 14165 , 693 , 479 , 385 , 1    , 1)
	#elif earth_fighter2.stat_level_get(stat_hp_current) < 0:
	#	earth_bugbear_5 = rnte(  499 , 476 , 14165 , 693 , 479 , 385 , 1    , 1)
	#if earth_fighter3 == OBJ_HANDLE_NULL:
	#	earth_bugbear_6 = rnte(  449 , 449 , 14163 , 694 , 481 , 391 , 4.9  , 1)
	#elif earth_fighter3.stat_level_get(stat_hp_current) < 0:
	#	earth_bugbear_6 = rnte(  449 , 449 , 14163 , 694 , 481 , 391 , 4.9  , 1)

	#earth_robe_guard4 = rnte(       445 , 444 , 14154 , 644 , 480 , 419 , 5.4  , 50)
	#earth_archer4 = rnte(           445 , 444 , 14154 , 644 , 480 , 419 , 5.4  , 50)
	#earth_archer5 = rnte(           445 , 444 , 14154 , 644 , 480 , 419 , 5.4  , 50)


	romagchest = rct(                445 , 444 , 1011  , 488 , 429 , 4)
	
	barr1 = game.obj_create( 121, location_from_axis(469, 379) )
	barr1.rotation = 0.8
	barr1.move(location_from_axis(469, 379), 10, 0 )
	barr1.portal_flag_set(OPF_JAMMED)
	
	barr2 = game.obj_create( 121, location_from_axis(467, 379) )
	barr2.rotation = 0.8
	barr2.move(location_from_axis(467, 379), 20, 0.69 )
	barr2.portal_flag_set(OPF_JAMMED)
	
	barr2_npc = game.obj_create( 14914, location_from_axis(467, 379) )
	barr2_npc.rotation = 0.8
	barr2_npc.move(location_from_axis(467, 379), 20, 1 )
	barr2_npc.fade_to(0,0, 255)
	barr2_npc.scripts[19] = 446 # heartbeat to bulletproof against PCs walking through the barrier
	
	barr3 = game.obj_create( 121, location_from_axis(466, 379) )
	barr3.rotation = 0.8
	barr3.move(location_from_axis(466, 379), 0, 1.46 )
	barr3.portal_flag_set(OPF_JAMMED)
	
	
	
	barr4 = game.obj_create( 121, location_from_axis(501, 378) )
	barr4.rotation = 0.8
	barr4.move(location_from_axis(501, 378), 39, 25 )
	barr4.portal_flag_set(OPF_JAMMED)
	
	barr5 = game.obj_create( 121, location_from_axis(499, 378) )
	barr5.rotation = 0.8
	barr5.move(location_from_axis(499, 378), 36, 25.898 )
	barr5.portal_flag_set(OPF_JAMMED)
	
	barr6 = game.obj_create( 121, location_from_axis(497, 378) )
	barr6.rotation = 0.65
	barr6.move(location_from_axis(497, 378), 32, 23.2 )
	barr6.portal_flag_set(OPF_JAMMED)
	barr6_npc.scripts[19] = 446 # heartbeat to bulletproof against PCs walking through the barrier
	
	barr6_npc = game.obj_create( 14914, location_from_axis(497, 378) )
	barr6_npc.rotation = 0.65
	barr6_npc.move(location_from_axis(497, 378), 32, 23.4 )
	barr6_npc.fade_to(0,0, 255)
	
	barr7 = game.obj_create( 121, location_from_axis(495, 378) )
	barr7.rotation = 0.45
	barr7.move(location_from_axis(495, 378), 29, 9.9 )
	barr7.portal_flag_set(OPF_JAMMED)
	return



def earth_reg_old():
	#see rnte() for parameter explanation
	#               (xxx, yyy, ID,   SP_ID,n_x, n_y, rot, radius)
	ogrechief = rnte(471, 537, 14248, 387, 475, 413, 2.5, 50)
	romag = rnte(445, 444, 8045, 688, 482, 417, 5.5, 50)
	hartsch = rnte(445, 444, 14154, 689, 480, 419, 5, 50)
	gnoll_leader = rnte(505, 534, 14066, 690, 495, 416, 0.4, 50)
	earth_lieutenant = rnte(442, 458, 14339, 391, 477, 422, 5.1, 5)
	earth_lieutenant.obj_set_int(obj_f_critter_strategy, 0)
	earth_ogre1 = rnte(517, 541, 14249, 358, 473, 425, 4.5, 2)
	earth_ogre2 = rnte(498, 531, 14249, 359, 497, 430, 6, 2)
	earth_barbarian_gnoll1 = rnte(508, 531, 14078, 360, 471, 419, 4, 2)
	earth_robe_guard1 = rnte(473, 404, 14337, 365, 480, 422, 4.7, 4)
	earth_robe_guard2 = rnte(482, 396, 14337, 366, 493, 423, 6, 4)
	earth_robe_guard3 = rnte(489, 402, 14337, 367, 474, 433, 4.2, 4)
	#earth_robe_guard4 = rnte(445, 444, 14154, 344, 480, 419, 5.4, 50)
	earth_fighter1 = rnte(439, 492, 14338, 692, 485, 387, 5.4, 1)
	earth_fighter2 = rnte(441, 490, 14338, 393, 477, 429, 5.4, 1)
	earth_fighter3 = rnte(444, 494, 14338, 394, 486, 410, 4.9, 1)
	earth_troop_commander = rnte(446, 470, 14156, 383, 497, 421, 6.1, 10)
	earth_archer1 = rnte(505, 530, 14164, 369, 497, 425, 5.4, 3)
	earth_archer2 = rnte(502, 473, 14164, 670, 480, 431, 5.4, 3)
	earth_archer3 = rnte(475, 542, 14164, 371, 493, 432, 5.4, 3)
	#earth_archer4 = rnte(445, 444, 14154, 344, 480, 419, 5.4, 50)
	#earth_archer5 = rnte(445, 444, 14154, 344, 480, 419, 5.4, 50)
	earth_elemental_medium_1 = rnte(493, 391, 14381, 384, 495, 412, 5.4, 2)
	earth_elemental_medium_2 = rnte(474, 392, 14381, 385, 467, 412, 4.3, 2)
	earth_elemental_large = rnte(483, 423, 14296, 386, 483, 423, 5.58, 2)
	earth_bugbear_1 = rnte(523, 414, 14163, 350, 479, 386, 4.3, 1)
	earth_bugbear_2 = rnte(440, 445, 14162, 351, 483, 419, 4.3, 1)
	earth_bugbear_3 = rnte(505, 473, 14163, 352, 479, 416, 4.3, 1)
	earth_sentry_fore1 = rnte(417, 441, 14163, 361, 495, 389, 5.4, 2)
	earth_sentry_fore2 = rnte(441, 449, 14163, 362, 472, 389, 5.5, 2)
	earth_sentry_back1 = rnte(426, 432, 14162, 363, 493, 403, 5.45, 2)
	earth_sentry_back2 = rnte(519, 416, 14162, 364, 471, 399, 5.54, 2)

	romagchest = rct(445, 444, 1011, 488, 429, 4)

	return

def air_reg():
	kelno = rnt(545, 497, 8092, 700, 480, 494, 1.5, 5)


	# The kitchen dwellers
	k1 = rnt(560, 468, 14161, 721, 501, 504, 4.2, 2)
	k2 = rnt(553, 469, 14216, 722, 477, 509, 2.3, 2)
	k3 = rnt(559, 465, 14159, 723, 495, 497, 4.1, 2)
	k4 = rnt(551, 474, 14184, 724, 470, 508, 3, 2)
	if k4 == OBJ_HANDLE_NULL:
		k4 = rnt(561, 470, 14184, 724, 470, 508, 3, 2)
	k5 = rnt(559, 464, 14185, 725, 497, 507, 3, 2)
	if k5 == OBJ_HANDLE_NULL:
		k5 = rnt(564, 472, 14185, 725, 497, 507, 3, 2)
	k6 = rnt(553, 480, 14216, 726, 485, 516, 2.35, 2) #greeter, SE door, female bugbear
	if k6 == OBJ_HANDLE_NULL:
		k6 = rnt(564, 469, 14216, 726, 485, 516, 2.35, 2)
	if k6 != OBJ_HANDLE_NULL:
		k6.scripts[13] = 445
		#k6.scripts[19] = 445
	k7 = rnt(552, 478, 14159, 727, 499, 494, 4, 2) #greeter, SW door, bugbear
	if k7 != OBJ_HANDLE_NULL:
		k7.scripts[13] = 445
		#k7.scripts[19] = 445
	k8 = rnt(561, 479, 14079, 728, 495, 490, 3.7, 2)
	k9 = rnt(561, 474, 14079, 729, 502, 513, 4, 2)
	k10 = rnt(558, 477, 14080, 730, 497, 511, 1, 2)
	k11 = rnt(564, 474, 14080, 731, 474, 502, 1, 2) #greeter, NE door, gnoll
	if k11 != OBJ_HANDLE_NULL:
		k11.scripts[13] = 445
		#k11.scripts[19] = 445
	k12 = rnt(564, 480, 14187, 732, 496, 513, 1, 2)
	if k12 == OBJ_HANDLE_NULL:
		k12 = rnt(557, 468, 14187, 732, 496, 513, 1, 2)
	k13 = rnt(558, 474, 14067, 733, 496, 515, 1, 2)
	k14 = rnt(563, 477, 14067, 734, 503, 510, 4.5, 2)
	k15 = rnt(558, 480, 14067, 735, 496, 485, 4, 2)
	#k16 = rnt(445, 444, 14185, 736, 482, 417, 5.5, 2)


	# The 12 bugbears just outside Kelno's office
	if k6 != OBJ_HANDLE_NULL:
		g1 = rnt(564, 492, 14159, 701, 477, 496, 1.3, 2)
	else:
		g1 = rnt(564, 492, 14159, 726, 485, 516, 2.35, 2) # SE sentry
		if g1 != OBJ_HANDLE_NULL:
			g1.scripts[13] = 445
			#g1.scripts[19] = 445
	g2 = rnt(556, 499, 14158, 702, 475, 506, 2, 2)
	g3 = rnt(570, 489, 14160, 703, 482, 500, 2, 2)
	g4 = rnt(570, 501, 14160, 704, 484, 498, 3, 2)
	g5 = rnt(568, 498, 14161, 705, 475, 497, 2, 2)
	g6 = rnt(565, 499, 14158, 706, 472, 499, 1.5, 2)
	g7 = rnt(558, 501, 14215, 707, 474, 489, 3, 2)
	g8 = rnt(566, 501, 14216, 708, 481, 487, 2, 2)
	if g8 == OBJ_HANDLE_NULL:
		g8 = rnt(563, 501, 14216, 708, 481, 487, 2, 2)
	if k7 != OBJ_HANDLE_NULL:
		g9 = rnt(567, 494, 14161, 709, 488, 496, 4, 2)
		if g9 == OBJ_HANDLE_NULL:
			g9 = rnt(560, 500, 14161, 709, 488, 496, 4, 2)
	else:
		g9 = rnt(567, 494, 14161, 727, 499, 494, 4, 2) # SW sentry
		if g9 == OBJ_HANDLE_NULL:
			g9 = rnt(560, 500, 14161, 727, 499, 494, 4, 2)
		if g9 != OBJ_HANDLE_NULL:
			g9.scripts[13] = 445
			#g9.scripts[19] = 445
	g10 = rnt(567, 490, 14159, 710, 491, 493, 4, 2)
	g11 = rnt(568, 492, 14161, 711, 484, 493, 3, 2)
	g12 = rnt(562, 490, 14161, 712, 487, 488, 4, 2)

	#Bugbear defectors
	if game.global_flags[108] == 1 or game.party[0].stat_level_get(stat_constitution) >= 50:
		d1 = rnt(566, 519, 14247, 713, 486, 495, 4.5, 2)
		d2 = rnt(559, 520, 14247, 714, 488, 492, 3.5, 2)
		d3 = rnt(561, 512, 14247, 715, 475, 492, 1.9, 2)
		d4 = rnt(556, 514, 14247, 716, 480, 516, 2.4, 2)
		d5_leader = rnt(549, 512, 14231, 717, 492, 486, 3.6, 5)
	
	#Air Elementals
	ae1 = rnt(498, 504, 14380, 718, 478, 494, 5.5, 3)
	ae2 = rnt(477, 504, 14380, 719, 483, 490, 5.5, 3)
	ae3_large = rnt(486, 493, 14292, 720, 495, 504, 5.5, 50)



	kelnochest = rct(541, 495, 1014, 475, 487, 2.9)

	for obj1 in game.obj_list_vicinity(location_from_axis(518, 485), OLC_PORTAL):
		x1, y1 = location_to_axis(obj1.location)
		if x1 == 518 and y1 == 485:
			obj1.obj_set_int(obj_f_secretdoor_flags, 0)
			if ( obj1.portal_flags_get() & OPF_LOCKED ):
				obj1.portal_flag_unset( OPF_LOCKED )

			#if not ( obj1.portal_flags_get() & OPF_OPEN ):
			#	obj1.portal_toggle_open()
			#	# obj1.portal_flag_set(OPF_OPEN) - didn't work
	return

def water_reg():
	belsornig = rnt(545, 538, 8091, 750, 550, 583, 0, 5)
	
	jugg_old = fsnc(545, 538, 1618, 20)
	jugg_old.object_flag_set(OF_DONTDRAW)
	juggernaut = game.obj_create(2110, lfa(539, 573))
	juggernaut.move(lfa(539, 573),0,0)
	juggernaut.rotation = 5.71
	#juggernaut = rst(545, 538, 1618, 539, 573, 5.5, 20)

	gar1 = rnt(557, 533, 14239, 752, 532, 566, 4, 3)
	gar2 = rnt(556, 548, 14239, 753, 536, 561, 2.5, 3)
	gar3 = rnt(522, 547, 14239, 754, 545, 562, 2.5, 3)
	gar4 = rnt(523, 534, 14239, 755, 547, 565, 1, 3)

	bug1 = rnt(553, 547, 14181, 757, 539, 564, 5.5, 3)
	if bug1 != OBJ_HANDLE_NULL:
		bug1.scripts[13] = 445 #enter combat
		bug1.scripts[19] = 445 #heartbeat
		bug2 = rnt(554, 533, 14181, 756, 531, 573, 5, 3)
	else:
		bug2 = rnt(554, 533, 14181, 757, 539, 564, 5.5, 3)
		if bug2 != OBJ_HANDLE_NULL:
			bug2.scripts[13] = 445
			bug2.scripts[19] = 445


	nalorrem = rnt(548, 543, 8028, 758, 546, 586, 5.5, 5)
	merrolan = rnt(548, 535, 8027, 759, 533, 584, 5, 5)

	if game.global_flags[108] == 0:
		defec1_leader = rnt(549, 512, 14231, 760, 547, 577, 0, 5)
		defec2 = rnt(566, 519, 14247, 761, 526, 581, 5, 2)
		defec3 = rnt(559, 520, 14247, 762, 531, 578, 5, 2)
		defec4 = rnt(561, 512, 14247, 763, 534, 577, 5.5, 2)
		defec5 = rnt(556, 514, 14247, 764, 537, 580, 6, 2)

	owlbear = rnt(512, 557, 14046, 765, 533, 568, 5, 5)
	kelleen = rnt(509, 562, 14225, 766, 531, 570, 5, 5)

	if game.global_flags[112] == 1 and game.global_flags[105] == 0: # convinced oohlgrist via dialogue, and Alrrem is not dead (failsafe)
		oohlgrist = rnt(483, 614, 14195, 767, 546, 568, 0, 5)
		troll1 = rnt(496, 619, 14262, 768, 544, 567, 1, 5)
		troll2 = rnt(473, 610, 14262, 769, 544, 571, 0.5, 5)

	snake1 = rnt(531, 589, 14375, 770, 532, 587, 4.6, 5)
	snake2 = rnt(535, 598, 14375, 771, 528, 584, -1.6, 5)
	snake3 = rnt(543, 599, 14375, 772, 553, 580, 0.5, 5)
	snake4 = rnt(549, 588, 14375, 773, 547, 583, 0, 5)
	
	belsornigchest = rct(551, 566, 1013, 554, 587, 5)
	return

def fire_reg():
	fire_bugbear_1 = rnt(          403 , 474 , 14167 , 870 , 418 , 487 , 1  , 1)
	fire_bugbear_2 = rnt(          408 , 474 , 14167 , 871 , 411 , 511 , 4  , 1)
	fire_bugbear_3 = rnt(          416 , 472 , 14168 , 872 , 424 , 498 , 1  , 1)
	fire_bugbear_4 = rnt(          419 , 473 , 14169 , 873 , 415 , 488 , 5  , 1)
	fire_bugbear_5 = rnt(          398 , 488 , 14169 , 874 , 415 , 499 , 2.5  , 1)
	fire_bugbear_6 = rnt(          421 , 515 , 14169 , 875 , 420 , 510 , 1  , 1)
	fire_bugbear_7 = rnt(          415 , 515 , 14169 , 876 , 420 , 501 , 2  , 1)
	#fire_bugbear_8 = rnt(          403 , 474 , 14169 , 877 , 418 , 487 , 1  , 1)
	#fire_bugbear_9 = rnt(          403 , 474 , 14169 , 878 , 418 , 487 , 1  , 1)
	alrrem = rnt(                  426 , 490 , 8047  , 879 , 419 , 494 , 1  , 1)
	fire_hydra = rnt(              463 , 557 , 14343  , 880 , 416 , 507 , 2.5  , 1)
	aern = rnt(                    461 , 563 , 14224  , 881 , 414 , 503 , 2  , 1)
	#if game.global_flags[118] == 1  or game.party[0].stat_level_get(stat_constitution) >= 50: # enable for testing with uber party
	if game.global_flags[118] == 1 and game.global_flags[107] == 0: # convinced oohlgrist via dialogue, and Alrrem is not dead (failsafe)
		oohlgrist = rnt(483, 614, 14195, 882, 413, 509, 3, 5)
		troll1 = rnt(496, 619, 14262, 883, 413, 483, 4, 5)
		troll2 = rnt(473, 610, 14262, 884, 414, 492, 3.5, 5)
	werewolf1 = rnt(440, 471, 14344, 885, 410, 483, 2.5, 1)
	werewolf2 = rnt(439, 468, 14344, 886, 421, 507, 0, 1)

	# the two below are due to disparities between standpoints and initial locations as indicated by MOB file: (wouldn't hurt to fix the MOB)
	if fire_bugbear_4 == OBJ_HANDLE_NULL:
		fire_bugbear_4 = rnt(          414 , 494 , 14169 , 873 , 415 , 488 , 5  , 1)
	if fire_bugbear_5 == OBJ_HANDLE_NULL:
		fire_bugbear_5 = rnt(          412 , 498 , 14169 , 874 , 415 , 499 , 2.5  , 1)
	return

def rnt(source_x, source_y, obj_name, new_standpoint_ID, new_x, new_y, new_rotation, radius):
	#Relocate NPC To...
	#source_x, source_y - where the object currently is
	#obj_name - self explanatory...
	#new_standpoint_ID - assign new standpoint from the jumppoint.tab file, so the NPC doesn't try to get back to its old location
	#new_x, new_y, new_rotation - where the object is transferred to, and which rotation it is given
	#radius - this is used to limit the range of detection of the critter, in case you want to transfer a specific one
	#(i.e. if there are two Earth Temple troops close together but you want to pick a particular one, then use a small radius)
	transferee = fnnc(source_x, source_y, obj_name, radius)
	if transferee != OBJ_HANDLE_NULL:
		if critter_is_unconscious(transferee) == 0:
			sps(transferee, new_standpoint_ID)
			transferee.npc_flag_unset(ONF_WAYPOINTS_DAY)
			transferee.npc_flag_unset(ONF_WAYPOINTS_NIGHT)
			transferee.move(lfa(new_x, new_y),0,0)
			transferee.npc_flag_set(ONF_KOS_OVERRIDE)
			transferee.rotation = new_rotation
	return transferee

def rnte(source_x, source_y, obj_name, new_standpoint_ID, new_x, new_y, new_rotation, radius, extra_command, reserve_list):
	#Relocate NPC To... EARTH TEMPLE VARIANT

	#source_x, source_y - where the object currently is
	#obj_name - self explanatory...
	#new_standpoint_ID - assign new standpoint from the jumppoint.tab file, so the NPC doesn't try to get back to its old location
	#new_x, new_y, new_rotation - where the object is transferred to, and which rotation it is given
	#radius - this is used to limit the range of detection of the critter, in case you want to transfer a specific one
	#(i.e. if there are two Earth Temple troops close together but you want to pick a particular one, then use a small radius)

	# If the 'source NPC' is found, and it is conscious, it will be transferred.
	# Else, the reserve list is used.

	transferee = fnnc(source_x, source_y, obj_name, radius)
	if transferee != OBJ_HANDLE_NULL:
		if critter_is_unconscious(transferee) == 0: # NB: OBJ_HANDLE_NULL can't be checked for unconsciousness, it would fuck up the script
			if extra_command == 'lieutenant':
				transferee.obj_set_int(obj_f_critter_strategy, 0)

			sps(transferee, new_standpoint_ID)
			transferee.npc_flag_unset(ONF_WAYPOINTS_DAY)
			transferee.npc_flag_unset(ONF_WAYPOINTS_NIGHT)
			transferee.move(lfa(new_x, new_y),0,0)
			transferee.npc_flag_set(ONF_KOS_OVERRIDE)
			transferee.rotation = new_rotation
			transferee.obj_set_int(obj_f_npc_pad_i_4, new_standpoint_ID)
			if extra_command == 'melee' or extra_command == 'big':
				transferee.scripts[13] = 446 # Enter Combat
				transferee.scripts[14] = 446 # Exit Combat
				transferee.scripts[15] = 446 # Start Combat (round)
		elif (extra_command == 'melee' or extra_command == 'ranged') and len(reserve_list) > 0 :
			if len(reserve_list[0]) == 3: # Search radius is not specified - assume accurate entry (set search radius = 1)
				transferee = fnnc(reserve_list[0][0], reserve_list[0][1], reserve_list[0][2], 1)
				reserve_list = reserve_list[1:len(reserve_list)]
			else:
				transferee = fnnc(reserve_list[0][0], reserve_list[0][1], reserve_list[0][2], reserve_list[0][3])
				reserve_list = reserve_list[1:len(reserve_list)]
			sps(transferee, new_standpoint_ID)
			transferee.npc_flag_unset(ONF_WAYPOINTS_DAY)
			transferee.npc_flag_unset(ONF_WAYPOINTS_NIGHT)
			transferee.move(lfa(new_x, new_y),0,0)
			transferee.npc_flag_set(ONF_KOS_OVERRIDE)
			transferee.rotation = new_rotation
			transferee.obj_set_int(obj_f_npc_pad_i_4, new_standpoint_ID)
			if extra_command == 'melee':
				transferee.scripts[13] = 446 # Enter Combat
				transferee.scripts[14] = 446 # Exit Combat
				transferee.scripts[15] = 446 # Start Combat (round)
	elif (extra_command == 'melee' or extra_command == 'ranged') and len(reserve_list) > 0 : # I assume invalids have already been trimmed from the list
		if len(reserve_list[0]) == 3:
			transferee = fnnc(reserve_list[0][0], reserve_list[0][1], reserve_list[0][2], 1)
			reserve_list = reserve_list[1:len(reserve_list)]
		else:
			transferee = fnnc(reserve_list[0][0], reserve_list[0][1], reserve_list[0][2], reserve_list[0][3])
			reserve_list = reserve_list[1:len(reserve_list)]
		sps(transferee, new_standpoint_ID)
		transferee.npc_flag_unset(ONF_WAYPOINTS_DAY)
		transferee.npc_flag_unset(ONF_WAYPOINTS_NIGHT)
		transferee.move(lfa(new_x, new_y),0,0)
		transferee.npc_flag_set(ONF_KOS_OVERRIDE)
		transferee.rotation = new_rotation
		transferee.obj_set_int(obj_f_npc_pad_i_4, new_standpoint_ID)
		if extra_command == 'melee':
			transferee.scripts[13] = 446 # Enter Combat
			transferee.scripts[14] = 446 # Exit Combat
			transferee.scripts[15] = 446 # Start Combat (round)
	return reserve_list, transferee

def trim_dead(untrimmed_list):
	trimmed_list = []
	pp = 0
	while pp < len(untrimmed_list):
		if len(untrimmed_list[pp]) == 3:
			candidate = fnnc(untrimmed_list[pp][0], untrimmed_list[pp][1], untrimmed_list[pp][2], 1)
		elif len(untrimmed_list[pp]) == 4:
			candidate = fnnc(untrimmed_list[pp][0], untrimmed_list[pp][1], untrimmed_list[pp][2], untrimmed_list[pp][3])
		if candidate != OBJ_HANDLE_NULL:
			if critter_is_unconscious(candidate) == 0:
				trimmed_list = trimmed_list + [untrimmed_list[pp]]
		pp = pp + 1
	return trimmed_list


def rct(script_x, script_y, obj_name, new_x, new_y, new_rotation):
	# Relocate Container To...
	transferee = fcnc(script_x, script_y, obj_name)
	if transferee != OBJ_HANDLE_NULL:
		transferee.move(lfa(new_x, new_y),0,0)
		transferee.rotation = new_rotation
	return transferee

def rst(script_x, script_y, obj_name, new_x, new_y, new_rotation, radius):
	## Relocate Scenery To...
	transferee = fsnc(script_x, script_y, obj_name, radius)
	if transferee != OBJ_HANDLE_NULL:
		transferee.move(lfa(new_x, new_y),0,0)
		transferee.rotation = new_rotation
	return transferee


def sps(object_to_be_transferred,new_standpoint_ID):
	## standpoint set
	object_to_be_transferred.standpoint_set(STANDPOINT_DAY, new_standpoint_ID)
	object_to_be_transferred.standpoint_set(STANDPOINT_NIGHT, new_standpoint_ID)
	return
	

def fnnc( xx, yy, name, radius = 1 ): 
	# Find NPC near coordinate, detection radius optional
	for npc in game.obj_list_vicinity( lfa(xx,yy), OLC_NPC ):
		npc_x, npc_y = lta(npc.location)
		dist = sqrt((npc_x-xx)*(npc_x-xx) + (npc_y-yy)*(npc_y-yy))
		if (npc.name == name and dist <= radius):
			return npc
	return OBJ_HANDLE_NULL

def fcnc( xx,yy, name ): 
	## Find container near coordinate
	for container in game.obj_list_vicinity( lfa(xx,yy), OLC_CONTAINER ):
		if (container.name == name):
			return container
	return OBJ_HANDLE_NULL

def fsnc( xx,yy, name, radius ): 
	## Find scenery near coordinate
	for mang in game.obj_list_vicinity( lfa(xx,yy), OLC_SCENERY ):
		mang_x, mang_y = lta(mang.location)
		dist = sqrt((mang_x-xx)*(mang_x-xx) + (mang_y-yy)*(mang_y-yy))
		if (mang.name == name and dist <= radius):
			return mang
	return OBJ_HANDLE_NULL

def vlist():
	moshe = game.obj_list_vicinity(game.leader.location, OLC_NPC)
	return moshe

def lfa( x, y ):
	# initialize loc to be a LONG integer
	loc = 0L + y
	loc = ( loc << 32 ) + x
	return loc

def lta( loc ):
	if type(loc) == type(OBJ_HANDLE_NULL):
		loc = loc.location
	y = loc >> 32
	x = loc & 4294967295
	return ( x, y )

def spawn(prot,x,y):
	moshe = game.obj_create(prot,lfa(x,y))
	if (moshe != OBJ_HANDLE_NULL):
		return moshe
	return OBJ_HANDLE_NULL
