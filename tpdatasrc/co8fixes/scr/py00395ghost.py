from toee import *
from utilities import *
from Co8 import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	san_start_combat(attachee, triggerer)
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.name == 14662 or attachee.name == 14663):
	# undead legion ghosts
		if (attachee.map == 5121):
		# verbobonc exterior
			if (game.quests[83].state == qs_completed):
				attachee.object_flag_set(OF_OFF)
	elif (is_daytime() == 0):
	# is nighttime
		if (game.global_vars[765] >= 1):
		# player has encountered Moathouse Ambush at any of the 3 locations, meaning they have killed Turuko, Zert, and Kobort and their ghosts will haunt them
			if (attachee.name == 8699):
			# turuko ghost
				if (attachee.map == 5146):
				# castle level 4 - upper hall
					if (game.global_vars[696] == 0):
					# turuko ghost not activated
						attachee.object_flag_unset(OF_OFF)
						# turn on turuko ghost
						attachee.float_line(1000,triggerer)
						# turuko ghost screeches!
						game.global_vars[696] = 1
						# turuko ghost is now on
						game.global_flags[869] = 1
						# castle sleep impossible flag set
					elif (game.global_vars[696] == 6):
					# kobort ghost has made his following speech and gone away
						if anyone( triggerer.group_list(), "has_item", 12612 ) and anyone( triggerer.group_list(), "has_item", 12614 ) and anyone( triggerer.group_list(), "has_item", 12616 ):
						# player has all the ghosts' parts
							attachee.object_flag_unset(OF_OFF)
							# turn on turuko ghost
							attachee.float_line(1000,triggerer)
							# turuko ghost screeches!
							game.global_vars[696] = 7
							# turuko ghost is now on
			elif (attachee.name == 8859):
			# zert ghost
				if (attachee.map == 5121):
				# verbo exterior - around castle
					if (game.global_vars[696] == 2):
					# turuko ghost has made his opening speech and gone away
						attachee.object_flag_unset(OF_OFF)
						# turn on zert ghost
						attachee.float_line(2000,triggerer)
						# zert ghost screeches!
						game.global_vars[696] = 3
						# zert ghost is now on
					elif (game.global_vars[696] == 8):
					# turuko ghost has made his concluding speech and gone away
						attachee.object_flag_unset(OF_OFF)
						# turn on zert ghost
						undead_legion(attachee, triggerer)
						# zert ghost spawns the undead legion
						attachee.object_flag_set(OF_OFF)
						# turn off zert ghost
			elif (attachee.name == 8860):
			# kobort ghost
				if (attachee.map == 5143):
				# castle level 1 - basement
					if (game.global_vars[696] == 4):
					# zert ghost has made his following speech and gone away
						attachee.object_flag_unset(OF_OFF)
						# turn on kobort ghost
						attachee.float_line(3000,triggerer)
						# kobort ghost moans!
						game.global_vars[696] = 5
						# kobort ghost is now on
						game.global_flags[869] = 1
						# castle sleep impossible flag set
	elif (is_daytime() == 1):
	# is daytime
		attachee.object_flag_set(OF_OFF)
		# turn ghosts off because they only roll at night
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
		if (attachee.name == 8699):
		# turuko ghost
			if (attachee.map == 5146):
			# castle level 4 - upper hall
				print('Fucking hell')
				if (game.global_vars[696] == 1):
				# turuko ghost activated
					StopCombat(attachee, 0)
					game.party[0].begin_dialog( attachee, 100 )
					# turuko opening remarks, wants head back
					return RUN_DEFAULT
				elif (game.global_vars[696] == 7):
				# turuko ghost reactivated
					StopCombat(attachee, 0)
					game.party[0].begin_dialog( attachee, 1 )
					# turuko concluding remarks, got their stuff
					return RUN_DEFAULT
		elif (attachee.name == 8859):
		# zert ghost
			if (attachee.map == 5121):
			# verbo exterior - around castle
				if (game.global_vars[696] == 3):
				# zert ghost activated
					StopCombat(attachee, 0)
					game.party[0].begin_dialog( attachee, 200 )
					# zert following remarks, wants hands back
					return RUN_DEFAULT
		elif (attachee.name == 8860):
		# kobort ghost
			if (attachee.map == 5143):
			# castle level 1 - basement
				if (game.global_vars[696] == 5):
				# kobort ghost activated
					StopCombat(attachee, 0)
					game.party[0].begin_dialog( attachee, 300 )
					# kobort following remarks, wants feet back
					return RUN_DEFAULT
		else:
		# random rest attacking ghosts
			StopCombat(attachee, 0)
			attachee.float_line(4010,triggerer)
			# generic screech
			attachee.object_flag_set(OF_OFF)
			return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.global_vars[696] >= 1):
		if (attachee.name == 8699):
		# turuko ghost
			if (attachee.map == 5146):
			# castle level 4 - upper hall
				if (not game.combat_is_active()):
					for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
						if (is_better_to_talk(attachee,obj)):
							attachee.float_line(1010,triggerer)
							# turuko ghost screeches!
							game.new_sid = 0
		elif (attachee.name == 8859):
		# zert ghost
			if (attachee.map == 5121):
			# verbo exterior - around castle
				if (not game.combat_is_active()):
					for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
						if (is_cool_to_talk(attachee,obj)):
							attachee.float_line(2010,triggerer)
							# zert ghost screeches!
							game.new_sid = 0
		elif (attachee.name == 8860):
		# kobort ghost
			if (attachee.map == 5143):
			# castle level 1 - basement
				if (not game.combat_is_active()):
					for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
						if (is_better_to_talk(attachee,obj)):
							attachee.float_line(3010,triggerer)
							# kobort ghost moans!
							game.new_sid = 0
		elif (attachee.name == 14819):
			attachee.destroy()
	return RUN_DEFAULT


def is_better_to_talk(speaker,listener):
	if (speaker.distance_to(listener) <= 10):
		return 1
	return 0


def is_cool_to_talk(speaker,listener):
	if (speaker.distance_to(listener) <= 25):
		return 1
	return 0


def go_ghost( attachee, triggerer ):
	if (attachee.name == 8699):
	# turuko ghost
		if (attachee.map == 5146):
		# castle level 4 - upper hall
			if (game.global_vars[696] == 7):
				game.global_vars[696] = 8
				# increment var to turuko off
				attachee.object_flag_set(OF_OFF)
				# turn turuko ghost off
				game.particles( "hit-SUBDUAL-medium", attachee )
				# play particles
				game.sound( 4114, 1 )
				# play sound
			else:
				game.global_vars[696] = 2
				# increment var to turuko off
				attachee.object_flag_set(OF_OFF)
				# turn turuko ghost off
				game.particles( "hit-SUBDUAL-medium", attachee )
				# play particles
				game.sound( 4114, 1 )
				# play sound
				game.global_flags[869] = 0
				# castle sleep impossible flag unset
	elif (attachee.name == 8859):
	# zert ghost
		if (attachee.map == 5121):
		# verbo exterior - around castle
			game.global_vars[696] = 4
			# increment var to zert off
			attachee.object_flag_set(OF_OFF)
			# turn zert ghost off
			game.particles( "hit-SUBDUAL-medium", attachee )
			# play particles
			game.sound( 4114, 1 )
			# play sound
	elif (attachee.name == 8860):
	# kobort ghost
		if (attachee.map == 5143):
		# castle level 1 - basement
			game.global_vars[696] = 6
			# increment var to kobort off
			attachee.object_flag_set(OF_OFF)
			# turn kobort ghost off
			game.particles( "hit-SUBDUAL-medium", attachee )
			# play particles
			game.sound( 4114, 1 )
			# play sound
			game.global_flags[869] = 0
			# castle sleep impossible flag unset
	return RUN_DEFAULT


def dump_parts( attachee, triggerer ):
	party_transfer_to( attachee, 12612 )
	party_transfer_to( attachee, 12614 )
	party_transfer_to( attachee, 12616 )
	return RUN_DEFAULT


def undead_legion(attachee, triggerer):
	q01 = game.obj_create(14662, location_from_axis(732, 393))
	q01.rotation = 1.5
	game.timevent_add( bye_bye, ( q01, triggerer ), 30300 )
	q02 = game.obj_create(14663, location_from_axis(736, 393))
	q02.rotation = 2.5
	game.timevent_add( bye_bye, ( q02, triggerer ), 30300 )
	q03 = game.obj_create(14663, location_from_axis(740, 393))
	q03.rotation = 3.5
	game.timevent_add( bye_bye, ( q03, triggerer ), 30300 )
	q04 = game.obj_create(14663, location_from_axis(744, 393))
	q04.rotation = 4.5
	game.timevent_add( bye_bye, ( q04, triggerer ), 30300 )
	q05 = game.obj_create(14663, location_from_axis(748, 397))
	q05.rotation = 5.5
	game.timevent_add( bye_bye, ( q05, triggerer ), 30300 )
	q06 = game.obj_create(14662, location_from_axis(740, 389))
	q06.rotation = 0.5
	game.timevent_add( bye_bye, ( q06, triggerer ), 30300 )
	q07 = game.obj_create(14663, location_from_axis(732, 389))
	q07.rotation = 1.5
	game.timevent_add( bye_bye, ( q07, triggerer ), 30300 )
	q08 = game.obj_create(14663, location_from_axis(728, 389))
	q08.rotation = 2.5
	game.timevent_add( bye_bye, ( q08, triggerer ), 30300 )
	q09 = game.obj_create(14663, location_from_axis(724, 389))
	q09.rotation = 3.5
	game.timevent_add( bye_bye, ( q09, triggerer ), 30300 )
	q10 = game.obj_create(14663, location_from_axis(720, 389))
	q10.rotation = 4.5
	game.timevent_add( bye_bye, ( q10, triggerer ), 30300 )
	q11 = game.obj_create(14662, location_from_axis(716, 389))
	q11.rotation = 5.5
	game.timevent_add( bye_bye, ( q11, triggerer ), 30300 )
	q12 = game.obj_create(14663, location_from_axis(712, 389))
	q12.rotation = 0.5
	game.timevent_add( bye_bye, ( q12, triggerer ), 30000 )
	q13 = game.obj_create(14663, location_from_axis(708, 389))
	q13.rotation = 1.5
	game.timevent_add( bye_bye, ( q13, triggerer ), 30300 )
	q14 = game.obj_create(14663, location_from_axis(704, 389))
	q14.rotation = 2.5
	game.timevent_add( bye_bye, ( q14, triggerer ), 30300 )
	q15 = game.obj_create(14663, location_from_axis(700, 389))
	q15.rotation = 3.5
	game.timevent_add( bye_bye, ( q15, triggerer ), 30300 )
	q16 = game.obj_create(14662, location_from_axis(704, 393))
	q16.rotation = 4.5
	game.timevent_add( bye_bye, ( q16, triggerer ), 30300 )
	q17 = game.obj_create(14663, location_from_axis(696, 389))
	q17.rotation = 5.5
	game.timevent_add( bye_bye, ( q17, triggerer ), 30300 )
	q18 = game.obj_create(14663, location_from_axis(732, 385))
	q18.rotation = 0.5
	game.timevent_add( bye_bye, ( q18, triggerer ), 30300 )
	q19 = game.obj_create(14663, location_from_axis(728, 385))
	q19.rotation = 1.5
	game.timevent_add( bye_bye, ( q19, triggerer ), 30300 )
	q20 = game.obj_create(14663, location_from_axis(724, 385))
	q20.rotation = 2.5
	game.timevent_add( bye_bye, ( q20, triggerer ), 30300 )
	q21 = game.obj_create(14662, location_from_axis(720, 385))
	q21.rotation = 3.5
	game.timevent_add( bye_bye, ( q21, triggerer ), 30300 )
	q22 = game.obj_create(14663, location_from_axis(716, 385))
	q22.rotation = 4.5
	game.timevent_add( bye_bye, ( q22, triggerer ), 30300 )
	q23 = game.obj_create(14663, location_from_axis(712, 385))
	q23.rotation = 5.5
	game.timevent_add( bye_bye, ( q23, triggerer ), 30300 )
	q24 = game.obj_create(14663, location_from_axis(708, 385))
	q24.rotation = 0.5
	game.timevent_add( bye_bye, ( q24, triggerer ), 30300 )
	q25 = game.obj_create(14663, location_from_axis(704, 385))
	q25.rotation = 1.5
	game.timevent_add( bye_bye, ( q25, triggerer ), 30300 )
	q26 = game.obj_create(14662, location_from_axis(700, 385))
	q26.rotation = 2.5
	game.timevent_add( bye_bye, ( q26, triggerer ), 30300 )
	q27 = game.obj_create(14663, location_from_axis(696, 385))
	q27.rotation = 3.5
	game.timevent_add( bye_bye, ( q27, triggerer ), 30300 )
	q28 = game.obj_create(14663, location_from_axis(692, 385))
	q28.rotation = 4.5
	game.timevent_add( bye_bye, ( q28, triggerer ), 30300 )
	q29 = game.obj_create(14663, location_from_axis(744, 389))
	q29.rotation = 5.5
	game.timevent_add( bye_bye, ( q29, triggerer ), 30300 )
	q30 = game.obj_create(14663, location_from_axis(748, 393))
	q30.rotation = 0.5
	game.timevent_add( bye_bye, ( q30, triggerer ), 30300 )
	r01 = game.obj_create(14662, location_from_axis(680, 449))
	r01.rotation = 1.5
	game.timevent_add( bye_bye, ( r01, triggerer ), 30300 )
	r02 = game.obj_create(14663, location_from_axis(680, 453))
	r02.rotation = 2.5
	game.timevent_add( bye_bye, ( r02, triggerer ), 30300 )
	r03 = game.obj_create(14663, location_from_axis(680, 457))
	r03.rotation = 3.5
	game.timevent_add( bye_bye, ( r03, triggerer ), 30300 )
	r04 = game.obj_create(14663, location_from_axis(680, 461))
	r04.rotation = 4.5
	game.timevent_add( bye_bye, ( r04, triggerer ), 30300 )
	r05 = game.obj_create(14663, location_from_axis(680, 465))
	r05.rotation = 5.5
	game.timevent_add( bye_bye, ( r05, triggerer ), 30300 )
	r06 = game.obj_create(14662, location_from_axis(680, 469))
	r06.rotation = 0.5
	game.timevent_add( bye_bye, ( r06, triggerer ), 30300 )
	r07 = game.obj_create(14663, location_from_axis(680, 473))
	r07.rotation = 1.5
	game.timevent_add( bye_bye, ( r07, triggerer ), 30300 )
	r08 = game.obj_create(14663, location_from_axis(740, 409))
	r08.rotation = 2.5
	game.timevent_add( bye_bye, ( r08, triggerer ), 30300 )
	r09 = game.obj_create(14663, location_from_axis(740, 413))
	r09.rotation = 3.5
	game.timevent_add( bye_bye, ( r09, triggerer ), 30300 )
	r10 = game.obj_create(14663, location_from_axis(752, 409))
	r10.rotation = 4.5
	game.timevent_add( bye_bye, ( r10, triggerer ), 30300 )
	r11 = game.obj_create(14662, location_from_axis(748, 405))
	r11.rotation = 5.5
	game.timevent_add( bye_bye, ( r11, triggerer ), 30300 )
	r12 = game.obj_create(14663, location_from_axis(752, 405))
	r12.rotation = 0.5
	game.timevent_add( bye_bye, ( r12, triggerer ), 30000 )
	r13 = game.obj_create(14663, location_from_axis(752, 401))
	r13.rotation = 1.5
	game.timevent_add( bye_bye, ( r13, triggerer ), 30300 )
	r14 = game.obj_create(14663, location_from_axis(736, 401))
	r14.rotation = 2.5
	game.timevent_add( bye_bye, ( r14, triggerer ), 30300 )
	r15 = game.obj_create(14663, location_from_axis(732, 401))
	r15.rotation = 3.5
	game.timevent_add( bye_bye, ( r15, triggerer ), 30300 )
	r16 = game.obj_create(14662, location_from_axis(728, 401))
	r16.rotation = 4.5
	game.timevent_add( bye_bye, ( r16, triggerer ), 30300 )
	r17 = game.obj_create(14663, location_from_axis(724, 401))
	r17.rotation = 5.5
	game.timevent_add( bye_bye, ( r17, triggerer ), 30300 )
	r18 = game.obj_create(14663, location_from_axis(732, 397))
	r18.rotation = 0.5
	game.timevent_add( bye_bye, ( r18, triggerer ), 30300 )
	r19 = game.obj_create(14663, location_from_axis(728, 397))
	r19.rotation = 1.5
	game.timevent_add( bye_bye, ( r19, triggerer ), 30300 )
	r20 = game.obj_create(14663, location_from_axis(724, 397))
	r20.rotation = 2.5
	game.timevent_add( bye_bye, ( r20, triggerer ), 30300 )
	r21 = game.obj_create(14662, location_from_axis(720, 397))
	r21.rotation = 3.5
	game.timevent_add( bye_bye, ( r21, triggerer ), 30300 )
	r22 = game.obj_create(14663, location_from_axis(716, 397))
	r22.rotation = 4.5
	game.timevent_add( bye_bye, ( r22, triggerer ), 30300 )
	r23 = game.obj_create(14663, location_from_axis(720, 401))
	r23.rotation = 5.5
	game.timevent_add( bye_bye, ( r23, triggerer ), 30300 )
	r24 = game.obj_create(14663, location_from_axis(712, 397))
	r24.rotation = 0.5
	game.timevent_add( bye_bye, ( r24, triggerer ), 30300 )
	r25 = game.obj_create(14663, location_from_axis(728, 393))
	r25.rotation = 1.5
	game.timevent_add( bye_bye, ( r25, triggerer ), 30300 )
	r26 = game.obj_create(14662, location_from_axis(724, 393))
	r26.rotation = 2.5
	game.timevent_add( bye_bye, ( r26, triggerer ), 30300 )
	r27 = game.obj_create(14663, location_from_axis(720, 393))
	r27.rotation = 3.5
	game.timevent_add( bye_bye, ( r27, triggerer ), 30300 )
	r28 = game.obj_create(14663, location_from_axis(716, 393))
	r28.rotation = 4.5
	game.timevent_add( bye_bye, ( r28, triggerer ), 30300 )
	r29 = game.obj_create(14663, location_from_axis(712, 393))
	r29.rotation = 5.5
	game.timevent_add( bye_bye, ( r29, triggerer ), 30300 )
	r30 = game.obj_create(14663, location_from_axis(708, 393))
	r30.rotation = 0.5
	game.timevent_add( bye_bye, ( r30, triggerer ), 30300 )
	s01 = game.obj_create(14662, location_from_axis(664, 417))
	s01.rotation = 1.5
	game.timevent_add( bye_bye, ( s01, triggerer ), 30300 )
	s02 = game.obj_create(14663, location_from_axis(664, 421))
	s02.rotation = 2.5
	game.timevent_add( bye_bye, ( s02, triggerer ), 30300 )
	s03 = game.obj_create(14663, location_from_axis(664, 425))
	s03.rotation = 3.5
	game.timevent_add( bye_bye, ( s03, triggerer ), 30300 )
	s04 = game.obj_create(14663, location_from_axis(664, 429))
	s04.rotation = 4.5
	game.timevent_add( bye_bye, ( s04, triggerer ), 30300 )
	s05 = game.obj_create(14663, location_from_axis(664, 437))
	s05.rotation = 5.5
	game.timevent_add( bye_bye, ( s05, triggerer ), 30300 )
	s06 = game.obj_create(14662, location_from_axis(664, 441))
	s06.rotation = 0.5
	game.timevent_add( bye_bye, ( s06, triggerer ), 30300 )
	s07 = game.obj_create(14663, location_from_axis(664, 449))
	s07.rotation = 1.5
	game.timevent_add( bye_bye, ( s07, triggerer ), 30300 )
	s08 = game.obj_create(14663, location_from_axis(664, 453))
	s08.rotation = 2.5
	game.timevent_add( bye_bye, ( s08, triggerer ), 30300 )
	s09 = game.obj_create(14663, location_from_axis(664, 457))
	s09.rotation = 3.5
	game.timevent_add( bye_bye, ( s09, triggerer ), 30300 )
	s10 = game.obj_create(14663, location_from_axis(664, 461))
	s10.rotation = 4.5
	game.timevent_add( bye_bye, ( s10, triggerer ), 30300 )
	s11 = game.obj_create(14662, location_from_axis(664, 465))
	s11.rotation = 5.5
	game.timevent_add( bye_bye, ( s11, triggerer ), 30300 )
	s12 = game.obj_create(14663, location_from_axis(664, 469))
	s12.rotation = 0.5
	game.timevent_add( bye_bye, ( s12, triggerer ), 30000 )
	s13 = game.obj_create(14663, location_from_axis(660, 421))
	s13.rotation = 1.5
	game.timevent_add( bye_bye, ( s13, triggerer ), 30300 )
	s14 = game.obj_create(14663, location_from_axis(660, 425))
	s14.rotation = 2.5
	game.timevent_add( bye_bye, ( s14, triggerer ), 30300 )
	s15 = game.obj_create(14663, location_from_axis(660, 429))
	s15.rotation = 3.5
	game.timevent_add( bye_bye, ( s15, triggerer ), 30300 )
	s16 = game.obj_create(14662, location_from_axis(660, 433))
	s16.rotation = 4.5
	game.timevent_add( bye_bye, ( s16, triggerer ), 30300 )
	s17 = game.obj_create(14663, location_from_axis(660, 437))
	s17.rotation = 5.5
	game.timevent_add( bye_bye, ( s17, triggerer ), 30300 )
	s18 = game.obj_create(14663, location_from_axis(660, 449))
	s18.rotation = 0.5
	game.timevent_add( bye_bye, ( s18, triggerer ), 30300 )
	s19 = game.obj_create(14663, location_from_axis(660, 453))
	s19.rotation = 1.5
	game.timevent_add( bye_bye, ( s19, triggerer ), 30300 )
	s20 = game.obj_create(14663, location_from_axis(660, 457))
	s20.rotation = 2.5
	game.timevent_add( bye_bye, ( s20, triggerer ), 30300 )
	s21 = game.obj_create(14662, location_from_axis(660, 461))
	s21.rotation = 3.5
	game.timevent_add( bye_bye, ( s21, triggerer ), 30300 )
	s22 = game.obj_create(14663, location_from_axis(660, 465))
	s22.rotation = 4.5
	game.timevent_add( bye_bye, ( s22, triggerer ), 30300 )
	s23 = game.obj_create(14663, location_from_axis(656, 425))
	s23.rotation = 5.5
	game.timevent_add( bye_bye, ( s23, triggerer ), 30300 )
	s24 = game.obj_create(14663, location_from_axis(656, 429))
	s24.rotation = 0.5
	game.timevent_add( bye_bye, ( s24, triggerer ), 30300 )
	s25 = game.obj_create(14663, location_from_axis(656, 433))
	s25.rotation = 1.5
	game.timevent_add( bye_bye, ( s25, triggerer ), 30300 )
	s26 = game.obj_create(14662, location_from_axis(656, 437))
	s26.rotation = 2.5
	game.timevent_add( bye_bye, ( s26, triggerer ), 30300 )
	s27 = game.obj_create(14663, location_from_axis(656, 441))
	s27.rotation = 3.5
	game.timevent_add( bye_bye, ( s27, triggerer ), 30300 )
	s28 = game.obj_create(14663, location_from_axis(656, 445))
	s28.rotation = 4.5
	game.timevent_add( bye_bye, ( s28, triggerer ), 30300 )
	s29 = game.obj_create(14663, location_from_axis(656, 449))
	s29.rotation = 5.5
	game.timevent_add( bye_bye, ( s29, triggerer ), 30300 )
	s30 = game.obj_create(14663, location_from_axis(656, 457))
	s30.rotation = 0.5
	game.timevent_add( bye_bye, ( s30, triggerer ), 30300 )
	t01 = game.obj_create(14662, location_from_axis(656, 461))
	t01.rotation = 1.5
	game.timevent_add( bye_bye, ( t01, triggerer ), 30300 )
	t02 = game.obj_create(14663, location_from_axis(676, 445))
	t02.rotation = 2.5
	game.timevent_add( bye_bye, ( t02, triggerer ), 30300 )
	t03 = game.obj_create(14663, location_from_axis(676, 449))
	t03.rotation = 3.5
	game.timevent_add( bye_bye, ( t03, triggerer ), 30300 )
	t04 = game.obj_create(14663, location_from_axis(676, 453))
	t04.rotation = 4.5
	game.timevent_add( bye_bye, ( t04, triggerer ), 30300 )
	t05 = game.obj_create(14663, location_from_axis(676, 457))
	t05.rotation = 5.5
	game.timevent_add( bye_bye, ( t05, triggerer ), 30300 )
	t06 = game.obj_create(14662, location_from_axis(676, 461))
	t06.rotation = 0.5
	game.timevent_add( bye_bye, ( t06, triggerer ), 30300 )
	t07 = game.obj_create(14663, location_from_axis(676, 465))
	t07.rotation = 1.5
	game.timevent_add( bye_bye, ( t07, triggerer ), 30300 )
	t08 = game.obj_create(14663, location_from_axis(676, 469))
	t08.rotation = 2.5
	game.timevent_add( bye_bye, ( t08, triggerer ), 30300 )
	t09 = game.obj_create(14663, location_from_axis(676, 473))
	t09.rotation = 3.5
	game.timevent_add( bye_bye, ( t09, triggerer ), 30300 )
	t10 = game.obj_create(14663, location_from_axis(680, 433))
	t10.rotation = 4.5
	game.timevent_add( bye_bye, ( t10, triggerer ), 30300 )
	t11 = game.obj_create(14662, location_from_axis(676, 433))
	t11.rotation = 5.5
	game.timevent_add( bye_bye, ( t11, triggerer ), 30300 )
	t12 = game.obj_create(14663, location_from_axis(672, 433))
	t12.rotation = 0.5
	game.timevent_add( bye_bye, ( t12, triggerer ), 30300 )
	t13 = game.obj_create(14663, location_from_axis(672, 437))
	t13.rotation = 1.5
	game.timevent_add( bye_bye, ( t13, triggerer ), 30300 )
	t14 = game.obj_create(14663, location_from_axis(672, 441))
	t14.rotation = 2.5
	game.timevent_add( bye_bye, ( t14, triggerer ), 30300 )
	t15 = game.obj_create(14663, location_from_axis(672, 445))
	t15.rotation = 3.5
	game.timevent_add( bye_bye, ( t15, triggerer ), 30300 )
	t16 = game.obj_create(14662, location_from_axis(672, 449))
	t16.rotation = 4.5
	game.timevent_add( bye_bye, ( t16, triggerer ), 30300 )
	t17 = game.obj_create(14663, location_from_axis(672, 453))
	t17.rotation = 5.5
	game.timevent_add( bye_bye, ( t17, triggerer ), 30300 )
	t18 = game.obj_create(14663, location_from_axis(672, 457))
	t18.rotation = 0.5
	game.timevent_add( bye_bye, ( t18, triggerer ), 30300 )
	t19 = game.obj_create(14663, location_from_axis(672, 461))
	t19.rotation = 1.5
	game.timevent_add( bye_bye, ( t19, triggerer ), 30300 )
	t20 = game.obj_create(14663, location_from_axis(672, 465))
	t20.rotation = 2.5
	game.timevent_add( bye_bye, ( t20, triggerer ), 30300 )
	t21 = game.obj_create(14662, location_from_axis(672, 469))
	t21.rotation = 3.5
	game.timevent_add( bye_bye, ( t21, triggerer ), 30300 )
	t22 = game.obj_create(14663, location_from_axis(672, 473))
	t22.rotation = 4.5
	game.timevent_add( bye_bye, ( t22, triggerer ), 30300 )
	t23 = game.obj_create(14663, location_from_axis(668, 421))
	t23.rotation = 5.5
	game.timevent_add( bye_bye, ( t23, triggerer ), 30300 )
	t24 = game.obj_create(14663, location_from_axis(668, 425))
	t24.rotation = 0.5
	game.timevent_add( bye_bye, ( t24, triggerer ), 30300 )
	t25 = game.obj_create(14663, location_from_axis(668, 429))
	t25.rotation = 1.5
	game.timevent_add( bye_bye, ( t25, triggerer ), 30300 )
	t26 = game.obj_create(14662, location_from_axis(668, 441))
	t26.rotation = 2.5
	game.timevent_add( bye_bye, ( t26, triggerer ), 30300 )
	t27 = game.obj_create(14663, location_from_axis(668, 445))
	t27.rotation = 3.5
	game.timevent_add( bye_bye, ( t27, triggerer ), 30300 )
	t28 = game.obj_create(14663, location_from_axis(668, 449))
	t28.rotation = 4.5
	game.timevent_add( bye_bye, ( t28, triggerer ), 30300 )
	t29 = game.obj_create(14663, location_from_axis(668, 461))
	t29.rotation = 5.5
	game.timevent_add( bye_bye, ( t29, triggerer ), 30300 )
	t30 = game.obj_create(14663, location_from_axis(668, 473))
	t30.rotation = 0.5
	game.timevent_add( bye_bye, ( t30, triggerer ), 30300 )
	u01 = game.obj_create(14662, location_from_axis(664, 413))
	u01.rotation = 1.5
	game.timevent_add( bye_bye, ( u01, triggerer ), 30300 )
	u02 = game.obj_create(14663, location_from_axis(740, 417))
	u02.rotation = 2.5
	game.timevent_add( bye_bye, ( u02, triggerer ), 30300 )
	u03 = game.obj_create(14663, location_from_axis(740, 421))
	u03.rotation = 3.5
	game.timevent_add( bye_bye, ( u03, triggerer ), 30300 )
	u04 = game.obj_create(14663, location_from_axis(740, 425))
	u04.rotation = 4.5
	game.timevent_add( bye_bye, ( u04, triggerer ), 30300 )
	u05 = game.obj_create(14663, location_from_axis(740, 429))
	u05.rotation = 5.5
	game.timevent_add( bye_bye, ( u05, triggerer ), 30300 )
	u06 = game.obj_create(14662, location_from_axis(740, 433))
	u06.rotation = 0.5
	game.timevent_add( bye_bye, ( u06, triggerer ), 30300 )
	u07 = game.obj_create(14663, location_from_axis(740, 437))
	u07.rotation = 1.5
	game.timevent_add( bye_bye, ( u07, triggerer ), 30300 )
	u08 = game.obj_create(14663, location_from_axis(740, 441))
	u08.rotation = 2.5
	game.timevent_add( bye_bye, ( u08, triggerer ), 30300 )
	u09 = game.obj_create(14663, location_from_axis(740, 445))
	u09.rotation = 3.5
	game.timevent_add( bye_bye, ( u09, triggerer ), 30300 )
	u10 = game.obj_create(14663, location_from_axis(740, 449))
	u10.rotation = 4.5
	game.timevent_add( bye_bye, ( u10, triggerer ), 30300 )
	u11 = game.obj_create(14662, location_from_axis(740, 453))
	u11.rotation = 5.5
	game.timevent_add( bye_bye, ( u11, triggerer ), 30300 )
	u12 = game.obj_create(14663, location_from_axis(740, 457))
	u12.rotation = 0.5
	game.timevent_add( bye_bye, ( u12, triggerer ), 30300 )
	u13 = game.obj_create(14663, location_from_axis(744, 413))
	u13.rotation = 1.5
	game.timevent_add( bye_bye, ( u13, triggerer ), 30300 )
	u14 = game.obj_create(14663, location_from_axis(744, 417))
	u14.rotation = 2.5
	game.timevent_add( bye_bye, ( u14, triggerer ), 30300 )
	u15 = game.obj_create(14663, location_from_axis(744, 421))
	u15.rotation = 3.5
	game.timevent_add( bye_bye, ( u15, triggerer ), 30300 )
	u16 = game.obj_create(14662, location_from_axis(744, 425))
	u16.rotation = 4.5
	game.timevent_add( bye_bye, ( u16, triggerer ), 30300 )
	u17 = game.obj_create(14663, location_from_axis(744, 429))
	u17.rotation = 5.5
	game.timevent_add( bye_bye, ( u17, triggerer ), 30300 )
	u18 = game.obj_create(14663, location_from_axis(744, 433))
	u18.rotation = 0.5
	game.timevent_add( bye_bye, ( u18, triggerer ), 30300 )
	u19 = game.obj_create(14663, location_from_axis(744, 437))
	u19.rotation = 1.5
	game.timevent_add( bye_bye, ( u19, triggerer ), 30300 )
	u20 = game.obj_create(14663, location_from_axis(744, 441))
	u20.rotation = 2.5
	game.timevent_add( bye_bye, ( u20, triggerer ), 30300 )
	u21 = game.obj_create(14662, location_from_axis(744, 445))
	u21.rotation = 3.5
	game.timevent_add( bye_bye, ( u21, triggerer ), 30300 )
	u22 = game.obj_create(14663, location_from_axis(748, 413))
	u22.rotation = 4.5
	game.timevent_add( bye_bye, ( u22, triggerer ), 30300 )
	u23 = game.obj_create(14663, location_from_axis(748, 417))
	u23.rotation = 5.5
	game.timevent_add( bye_bye, ( u23, triggerer ), 30300 )
	u24 = game.obj_create(14663, location_from_axis(748, 421))
	u24.rotation = 0.5
	game.timevent_add( bye_bye, ( u24, triggerer ), 30300 )
	u25 = game.obj_create(14663, location_from_axis(748, 425))
	u25.rotation = 1.5
	game.timevent_add( bye_bye, ( u25, triggerer ), 30300 )
	u26 = game.obj_create(14662, location_from_axis(748, 429))
	u26.rotation = 2.5
	game.timevent_add( bye_bye, ( u26, triggerer ), 30300 )
	u27 = game.obj_create(14663, location_from_axis(748, 433))
	u27.rotation = 3.5
	game.timevent_add( bye_bye, ( u27, triggerer ), 30300 )
	u28 = game.obj_create(14663, location_from_axis(748, 437))
	u28.rotation = 4.5
	game.timevent_add( bye_bye, ( u28, triggerer ), 30300 )
	u29 = game.obj_create(14663, location_from_axis(748, 441))
	u29.rotation = 5.5
	game.timevent_add( bye_bye, ( u29, triggerer ), 30300 )
	u30 = game.obj_create(14663, location_from_axis(752, 413))
	u30.rotation = 0.5
	game.timevent_add( bye_bye, ( u30, triggerer ), 30300 )
	v01 = game.obj_create(14662, location_from_axis(752, 417))
	v01.rotation = 1.5
	game.timevent_add( bye_bye, ( v01, triggerer ), 30300 )
	v02 = game.obj_create(14663, location_from_axis(752, 421))
	v02.rotation = 2.5
	game.timevent_add( bye_bye, ( v02, triggerer ), 30300 )
	v03 = game.obj_create(14663, location_from_axis(752, 425))
	v03.rotation = 3.5
	game.timevent_add( bye_bye, ( v03, triggerer ), 30300 )
	v04 = game.obj_create(14663, location_from_axis(752, 429))
	v04.rotation = 4.5
	game.timevent_add( bye_bye, ( v04, triggerer ), 30300 )
	v05 = game.obj_create(14663, location_from_axis(752, 433))
	v05.rotation = 5.5
	game.timevent_add( bye_bye, ( v05, triggerer ), 30300 )
	v06 = game.obj_create(14662, location_from_axis(752, 437))
	v06.rotation = 0.5
	game.timevent_add( bye_bye, ( v06, triggerer ), 30300 )
	v07 = game.obj_create(14663, location_from_axis(752, 441))
	v07.rotation = 1.5
	game.timevent_add( bye_bye, ( v07, triggerer ), 30300 )
	v08 = game.obj_create(14663, location_from_axis(736, 417))
	v08.rotation = 2.5
	game.timevent_add( bye_bye, ( v08, triggerer ), 30300 )
	v09 = game.obj_create(14663, location_from_axis(736, 429))
	v09.rotation = 3.5
	game.timevent_add( bye_bye, ( v09, triggerer ), 30300 )
	v10 = game.obj_create(14663, location_from_axis(736, 433))
	v10.rotation = 4.5
	game.timevent_add( bye_bye, ( v10, triggerer ), 30300 )
	v11 = game.obj_create(14662, location_from_axis(736, 437))
	v11.rotation = 5.5
	game.timevent_add( bye_bye, ( v11, triggerer ), 30300 )
	v12 = game.obj_create(14663, location_from_axis(736, 441))
	v12.rotation = 0.5
	game.timevent_add( bye_bye, ( v12, triggerer ), 30300 )
	v13 = game.obj_create(14663, location_from_axis(736, 445))
	v13.rotation = 1.5
	game.timevent_add( bye_bye, ( v13, triggerer ), 30300 )
	v14 = game.obj_create(14663, location_from_axis(736, 449))
	v14.rotation = 2.5
	game.timevent_add( bye_bye, ( v14, triggerer ), 30300 )
	v15 = game.obj_create(14663, location_from_axis(736, 453))
	v15.rotation = 3.5
	game.timevent_add( bye_bye, ( v15, triggerer ), 30300 )
	v16 = game.obj_create(14662, location_from_axis(736, 457))
	v16.rotation = 4.5
	game.timevent_add( bye_bye, ( v16, triggerer ), 30300 )
	v17 = game.obj_create(14663, location_from_axis(736, 461))
	v17.rotation = 5.5
	game.timevent_add( bye_bye, ( v17, triggerer ), 30300 )
	v18 = game.obj_create(14663, location_from_axis(732, 441))
	v18.rotation = 0.5
	game.timevent_add( bye_bye, ( v18, triggerer ), 30300 )
	v19 = game.obj_create(14663, location_from_axis(732, 445))
	v19.rotation = 1.5
	game.timevent_add( bye_bye, ( v19, triggerer ), 30300 )
	v20 = game.obj_create(14663, location_from_axis(732, 449))
	v20.rotation = 2.5
	game.timevent_add( bye_bye, ( v20, triggerer ), 30300 )
	v21 = game.obj_create(14662, location_from_axis(732, 453))
	v21.rotation = 3.5
	game.timevent_add( bye_bye, ( v21, triggerer ), 30300 )
	v22 = game.obj_create(14663, location_from_axis(732, 457))
	v22.rotation = 4.5
	game.timevent_add( bye_bye, ( v22, triggerer ), 30300 )
	v23 = game.obj_create(14663, location_from_axis(732, 461))
	v23.rotation = 5.5
	game.timevent_add( bye_bye, ( v23, triggerer ), 30300 )
	v24 = game.obj_create(14663, location_from_axis(732, 465))
	v24.rotation = 0.5
	game.timevent_add( bye_bye, ( v24, triggerer ), 30300 )
	v25 = game.obj_create(14663, location_from_axis(732, 469))
	v25.rotation = 1.5
	game.timevent_add( bye_bye, ( v25, triggerer ), 30300 )
	v26 = game.obj_create(14662, location_from_axis(732, 473))
	v26.rotation = 2.5
	game.timevent_add( bye_bye, ( v26, triggerer ), 30300 )
	v27 = game.obj_create(14663, location_from_axis(728, 445))
	v27.rotation = 3.5
	game.timevent_add( bye_bye, ( v27, triggerer ), 30300 )
	v28 = game.obj_create(14663, location_from_axis(728, 449))
	v28.rotation = 4.5
	game.timevent_add( bye_bye, ( v28, triggerer ), 30300 )
	v29 = game.obj_create(14663, location_from_axis(728, 453))
	v29.rotation = 5.5
	game.timevent_add( bye_bye, ( v29, triggerer ), 30300 )
	v30 = game.obj_create(14663, location_from_axis(728, 457))
	v30.rotation = 0.5
	game.timevent_add( bye_bye, ( v30, triggerer ), 30300 )
	w01 = game.obj_create(14662, location_from_axis(728, 461))
	w01.rotation = 1.5
	game.timevent_add( bye_bye, ( w01, triggerer ), 30300 )
	w02 = game.obj_create(14663, location_from_axis(728, 465))
	w02.rotation = 2.5
	game.timevent_add( bye_bye, ( w02, triggerer ), 30300 )
	w03 = game.obj_create(14663, location_from_axis(724, 445))
	w03.rotation = 3.5
	game.timevent_add( bye_bye, ( w03, triggerer ), 30300 )
	w04 = game.obj_create(14663, location_from_axis(724, 449))
	w04.rotation = 4.5
	game.timevent_add( bye_bye, ( w04, triggerer ), 30300 )
	w05 = game.obj_create(14663, location_from_axis(724, 453))
	w05.rotation = 5.5
	game.timevent_add( bye_bye, ( w05, triggerer ), 30300 )
	w06 = game.obj_create(14662, location_from_axis(724, 457))
	w06.rotation = 0.5
	game.timevent_add( bye_bye, ( w06, triggerer ), 30300 )
	w07 = game.obj_create(14663, location_from_axis(724, 461))
	w07.rotation = 1.5
	game.timevent_add( bye_bye, ( w07, triggerer ), 30300 )
	w08 = game.obj_create(14663, location_from_axis(724, 465))
	w08.rotation = 2.5
	game.timevent_add( bye_bye, ( w08, triggerer ), 30300 )
	w09 = game.obj_create(14663, location_from_axis(720, 449))
	w09.rotation = 3.5
	game.timevent_add( bye_bye, ( w09, triggerer ), 30300 )
	w10 = game.obj_create(14663, location_from_axis(720, 453))
	w10.rotation = 4.5
	game.timevent_add( bye_bye, ( w10, triggerer ), 30300 )
	w11 = game.obj_create(14662, location_from_axis(720, 457))
	w11.rotation = 5.5
	game.timevent_add( bye_bye, ( w11, triggerer ), 30300 )
	w12 = game.obj_create(14663, location_from_axis(720, 461))
	w12.rotation = 0.5
	game.timevent_add( bye_bye, ( w12, triggerer ), 30000 )
	w13 = game.obj_create(14663, location_from_axis(720, 465))
	w13.rotation = 1.5
	game.timevent_add( bye_bye, ( w13, triggerer ), 30300 )
	w14 = game.obj_create(14663, location_from_axis(720, 469))
	w14.rotation = 2.5
	game.timevent_add( bye_bye, ( w14, triggerer ), 30300 )
	w15 = game.obj_create(14663, location_from_axis(716, 449))
	w15.rotation = 3.5
	game.timevent_add( bye_bye, ( w15, triggerer ), 30300 )
	w16 = game.obj_create(14662, location_from_axis(716, 453))
	w16.rotation = 4.5
	game.timevent_add( bye_bye, ( w16, triggerer ), 30300 )
	w17 = game.obj_create(14663, location_from_axis(716, 457))
	w17.rotation = 5.5
	game.timevent_add( bye_bye, ( w17, triggerer ), 30300 )
	w18 = game.obj_create(14663, location_from_axis(716, 461))
	w18.rotation = 0.5
	game.timevent_add( bye_bye, ( w18, triggerer ), 30300 )
	w19 = game.obj_create(14663, location_from_axis(716, 465))
	w19.rotation = 1.5
	game.timevent_add( bye_bye, ( w19, triggerer ), 30300 )
	w20 = game.obj_create(14663, location_from_axis(716, 469))
	w20.rotation = 2.5
	game.timevent_add( bye_bye, ( w20, triggerer ), 30300 )
	w21 = game.obj_create(14662, location_from_axis(716, 473))
	w21.rotation = 3.5
	game.timevent_add( bye_bye, ( w21, triggerer ), 30300 )
	w22 = game.obj_create(14663, location_from_axis(716, 477))
	w22.rotation = 4.5
	game.timevent_add( bye_bye, ( w22, triggerer ), 30300 )
	w23 = game.obj_create(14663, location_from_axis(716, 481))
	w23.rotation = 5.5
	game.timevent_add( bye_bye, ( w23, triggerer ), 30300 )
	w24 = game.obj_create(14663, location_from_axis(716, 485))
	w24.rotation = 0.5
	game.timevent_add( bye_bye, ( w24, triggerer ), 30300 )
	w25 = game.obj_create(14663, location_from_axis(716, 489))
	w25.rotation = 1.5
	game.timevent_add( bye_bye, ( w25, triggerer ), 30300 )
	w26 = game.obj_create(14662, location_from_axis(716, 493))
	w26.rotation = 2.5
	game.timevent_add( bye_bye, ( w26, triggerer ), 30300 )
	w27 = game.obj_create(14663, location_from_axis(716, 497))
	w27.rotation = 3.5
	game.timevent_add( bye_bye, ( w27, triggerer ), 30300 )
	w28 = game.obj_create(14663, location_from_axis(712, 453))
	w28.rotation = 4.5
	game.timevent_add( bye_bye, ( w28, triggerer ), 30300 )
	w29 = game.obj_create(14663, location_from_axis(712, 457))
	w29.rotation = 5.5
	game.timevent_add( bye_bye, ( w29, triggerer ), 30300 )
	w30 = game.obj_create(14663, location_from_axis(712, 461))
	w30.rotation = 0.5
	game.timevent_add( bye_bye, ( w30, triggerer ), 30300 )
	x01 = game.obj_create(14662, location_from_axis(712, 465))
	x01.rotation = 1.5
	game.timevent_add( bye_bye, ( x01, triggerer ), 30300 )
	x02 = game.obj_create(14663, location_from_axis(708, 453))
	x02.rotation = 2.5
	game.timevent_add( bye_bye, ( x02, triggerer ), 30300 )
	x03 = game.obj_create(14663, location_from_axis(712, 473))
	x03.rotation = 3.5
	game.timevent_add( bye_bye, ( x03, triggerer ), 30300 )
	x04 = game.obj_create(14663, location_from_axis(712, 477))
	x04.rotation = 4.5
	game.timevent_add( bye_bye, ( x04, triggerer ), 30300 )
	x05 = game.obj_create(14663, location_from_axis(712, 481))
	x05.rotation = 5.5
	game.timevent_add( bye_bye, ( x05, triggerer ), 30300 )
	x06 = game.obj_create(14662, location_from_axis(712, 485))
	x06.rotation = 0.5
	game.timevent_add( bye_bye, ( x06, triggerer ), 30300 )
	x07 = game.obj_create(14663, location_from_axis(712, 489))
	x07.rotation = 1.5
	game.timevent_add( bye_bye, ( x07, triggerer ), 30300 )
	x08 = game.obj_create(14663, location_from_axis(712, 493))
	x08.rotation = 2.5
	game.timevent_add( bye_bye, ( x08, triggerer ), 30300 )
	x09 = game.obj_create(14663, location_from_axis(712, 497))
	x09.rotation = 3.5
	game.timevent_add( bye_bye, ( x09, triggerer ), 30300 )
	x10 = game.obj_create(14663, location_from_axis(712, 501))
	x10.rotation = 4.5
	game.timevent_add( bye_bye, ( x10, triggerer ), 30300 )
	x11 = game.obj_create(14662, location_from_axis(708, 457))
	x11.rotation = 5.5
	game.timevent_add( bye_bye, ( x11, triggerer ), 30300 )
	x12 = game.obj_create(14663, location_from_axis(708, 461))
	x12.rotation = 0.5
	game.timevent_add( bye_bye, ( x12, triggerer ), 30300 )
	x13 = game.obj_create(14663, location_from_axis(708, 469))
	x13.rotation = 1.5
	game.timevent_add( bye_bye, ( x13, triggerer ), 30300 )
	x14 = game.obj_create(14663, location_from_axis(708, 473))
	x14.rotation = 2.5
	game.timevent_add( bye_bye, ( x14, triggerer ), 30300 )
	x15 = game.obj_create(14663, location_from_axis(708, 477))
	x15.rotation = 3.5
	game.timevent_add( bye_bye, ( x15, triggerer ), 30300 )
	x16 = game.obj_create(14662, location_from_axis(708, 481))
	x16.rotation = 4.5
	game.timevent_add( bye_bye, ( x16, triggerer ), 30300 )
	x17 = game.obj_create(14663, location_from_axis(708, 485))
	x17.rotation = 5.5
	game.timevent_add( bye_bye, ( x17, triggerer ), 30300 )
	x18 = game.obj_create(14663, location_from_axis(708, 489))
	x18.rotation = 0.5
	game.timevent_add( bye_bye, ( x18, triggerer ), 30300 )
	x19 = game.obj_create(14663, location_from_axis(708, 493))
	x19.rotation = 1.5
	game.timevent_add( bye_bye, ( x19, triggerer ), 30300 )
	x20 = game.obj_create(14663, location_from_axis(708, 497))
	x20.rotation = 2.5
	game.timevent_add( bye_bye, ( x20, triggerer ), 30300 )
	x21 = game.obj_create(14662, location_from_axis(704, 457))
	x21.rotation = 3.5
	game.timevent_add( bye_bye, ( x21, triggerer ), 30300 )
	x22 = game.obj_create(14663, location_from_axis(704, 461))
	x22.rotation = 4.5
	game.timevent_add( bye_bye, ( x22, triggerer ), 30300 )
	x23 = game.obj_create(14663, location_from_axis(704, 469))
	x23.rotation = 5.5
	game.timevent_add( bye_bye, ( x23, triggerer ), 30300 )
	x24 = game.obj_create(14663, location_from_axis(704, 473))
	x24.rotation = 0.5
	game.timevent_add( bye_bye, ( x24, triggerer ), 30300 )
	x25 = game.obj_create(14663, location_from_axis(704, 477))
	x25.rotation = 1.5
	game.timevent_add( bye_bye, ( x25, triggerer ), 30300 )
	x26 = game.obj_create(14662, location_from_axis(704, 481))
	x26.rotation = 2.5
	game.timevent_add( bye_bye, ( x26, triggerer ), 30300 )
	x27 = game.obj_create(14663, location_from_axis(704, 485))
	x27.rotation = 3.5
	game.timevent_add( bye_bye, ( x27, triggerer ), 30300 )
	x28 = game.obj_create(14663, location_from_axis(704, 489))
	x28.rotation = 4.5
	game.timevent_add( bye_bye, ( x28, triggerer ), 30300 )
	x29 = game.obj_create(14663, location_from_axis(705, 493))
	x29.rotation = 5.5
	game.timevent_add( bye_bye, ( x29, triggerer ), 30300 )
	x30 = game.obj_create(14663, location_from_axis(748, 409))
	x30.rotation = 0.5
	game.timevent_add( bye_bye, ( x30, triggerer ), 30300 )
	y01 = game.obj_create(14662, location_from_axis(744, 409))
	y01.rotation = 1.5
	game.timevent_add( bye_bye, ( y01, triggerer ), 30300 )
	y02 = game.obj_create(14663, location_from_axis(720, 485))
	y02.rotation = 2.5
	game.timevent_add( bye_bye, ( y02, triggerer ), 30300 )
	y03 = game.obj_create(14663, location_from_axis(720, 489))
	y03.rotation = 3.5
	game.timevent_add( bye_bye, ( y03, triggerer ), 30300 )
	y04 = game.obj_create(14663, location_from_axis(720, 493))
	y04.rotation = 4.5
	game.timevent_add( bye_bye, ( y04, triggerer ), 30300 )
	y05 = game.obj_create(14663, location_from_axis(724, 493))
	y05.rotation = 5.5
	game.timevent_add( bye_bye, ( y05, triggerer ), 30300 )
	y06 = game.obj_create(14662, location_from_axis(728, 493))
	y06.rotation = 0.5
	game.timevent_add( bye_bye, ( y06, triggerer ), 30300 )
	y07 = game.obj_create(14663, location_from_axis(732, 489))
	y07.rotation = 1.5
	game.timevent_add( bye_bye, ( y07, triggerer ), 30300 )
	y08 = game.obj_create(14663, location_from_axis(736, 489))
	y08.rotation = 2.5
	game.timevent_add( bye_bye, ( y08, triggerer ), 30300 )
	y09 = game.obj_create(14663, location_from_axis(740, 489))
	y09.rotation = 3.5
	game.timevent_add( bye_bye, ( y09, triggerer ), 30300 )
	y10 = game.obj_create(14663, location_from_axis(740, 485))
	y10.rotation = 4.5
	game.timevent_add( bye_bye, ( y10, triggerer ), 30300 )
	y11 = game.obj_create(14662, location_from_axis(736, 481))
	y11.rotation = 5.5
	game.timevent_add( bye_bye, ( y11, triggerer ), 30300 )
	y12 = game.obj_create(14663, location_from_axis(736, 477))
	y12.rotation = 0.5
	game.timevent_add( bye_bye, ( y12, triggerer ), 30000 )
	y13 = game.obj_create(14663, location_from_axis(744, 485))
	y13.rotation = 1.5
	game.timevent_add( bye_bye, ( y13, triggerer ), 30300 )
	y14 = game.obj_create(14663, location_from_axis(748, 481))
	y14.rotation = 2.5
	game.timevent_add( bye_bye, ( y14, triggerer ), 30300 )
	y15 = game.obj_create(14663, location_from_axis(752, 481))
	y15.rotation = 3.5
	game.timevent_add( bye_bye, ( y15, triggerer ), 30300 )
	y16 = game.obj_create(14662, location_from_axis(752, 477))
	y16.rotation = 4.5
	game.timevent_add( bye_bye, ( y16, triggerer ), 30300 )
	y17 = game.obj_create(14663, location_from_axis(756, 477))
	y17.rotation = 5.5
	game.timevent_add( bye_bye, ( y17, triggerer ), 30300 )
	y18 = game.obj_create(14663, location_from_axis(756, 473))
	y18.rotation = 0.5
	game.timevent_add( bye_bye, ( y18, triggerer ), 30300 )
	y19 = game.obj_create(14663, location_from_axis(760, 469))
	y19.rotation = 1.5
	game.timevent_add( bye_bye, ( y19, triggerer ), 30300 )
	y20 = game.obj_create(14663, location_from_axis(760, 465))
	y20.rotation = 2.5
	game.timevent_add( bye_bye, ( y20, triggerer ), 30300 )
	y21 = game.obj_create(14662, location_from_axis(756, 465))
	y21.rotation = 3.5
	game.timevent_add( bye_bye, ( y21, triggerer ), 30300 )
	y22 = game.obj_create(14663, location_from_axis(748, 461))
	y22.rotation = 4.5
	game.timevent_add( bye_bye, ( y22, triggerer ), 30300 )
	y23 = game.obj_create(14663, location_from_axis(756, 425))
	y23.rotation = 5.5
	game.timevent_add( bye_bye, ( y23, triggerer ), 30300 )
	y24 = game.obj_create(14663, location_from_axis(756, 429))
	y24.rotation = 0.5
	game.timevent_add( bye_bye, ( y24, triggerer ), 30300 )
	y25 = game.obj_create(14663, location_from_axis(756, 433))
	y25.rotation = 1.5
	game.timevent_add( bye_bye, ( y25, triggerer ), 30300 )
	y26 = game.obj_create(14662, location_from_axis(756, 437))
	y25.rotation = 2.5
	game.timevent_add( bye_bye, ( y26, triggerer ), 30300 )
	y27 = game.obj_create(14663, location_from_axis(756, 441))
	y27.rotation = 3.5
	game.timevent_add( bye_bye, ( y27, triggerer ), 30300 )
	y28 = game.obj_create(14663, location_from_axis(756, 445))
	y28.rotation = 4.5
	game.timevent_add( bye_bye, ( y28, triggerer ), 30300 )
	y29 = game.obj_create(14663, location_from_axis(756, 421))
	y29.rotation = 5.5
	game.timevent_add( bye_bye, ( y29, triggerer ), 30300 )
	y30 = game.obj_create(14663, location_from_axis(760, 429))
	y30.rotation = 0.5
	game.timevent_add( bye_bye, ( y30, triggerer ), 30300 )
	z01 = game.obj_create(14662, location_from_axis(760, 433))
	z01.rotation = 1.5
	game.timevent_add( bye_bye, ( z01, triggerer ), 30300 )
	z02 = game.obj_create(14663, location_from_axis(760, 437))
	z02.rotation = 2.5
	game.timevent_add( bye_bye, ( z02, triggerer ), 30300 )
	z03 = game.obj_create(14663, location_from_axis(760, 441))
	z03.rotation = 3.5
	game.timevent_add( bye_bye, ( z03, triggerer ), 30300 )
	z04 = game.obj_create(14663, location_from_axis(760, 445))
	z04.rotation = 4.5
	game.timevent_add( bye_bye, ( z04, triggerer ), 30300 )
	z05 = game.obj_create(14663, location_from_axis(764, 433))
	z05.rotation = 5.5
	game.timevent_add( bye_bye, ( z05, triggerer ), 30300 )
	z06 = game.obj_create(14662, location_from_axis(764, 437))
	z06.rotation = 0.5
	game.timevent_add( bye_bye, ( z06, triggerer ), 30300 )
	z07 = game.obj_create(14663, location_from_axis(764, 441))
	z07.rotation = 1.5
	game.timevent_add( bye_bye, ( z07, triggerer ), 30300 )
	z08 = game.obj_create(14663, location_from_axis(764, 445))
	z08.rotation = 2.5
	game.timevent_add( bye_bye, ( z08, triggerer ), 30300 )
	z09 = game.obj_create(14663, location_from_axis(764, 449))
	z09.rotation = 3.5
	game.timevent_add( bye_bye, ( z09, triggerer ), 30300 )
	z10 = game.obj_create(14663, location_from_axis(764, 461))
	z10.rotation = 4.5
	game.timevent_add( bye_bye, ( z10, triggerer ), 30300 )
	z11 = game.obj_create(14662, location_from_axis(764, 457))
	z11.rotation = 5.5
	game.timevent_add( bye_bye, ( z11, triggerer ), 30300 )
	z12 = game.obj_create(14663, location_from_axis(768, 433))
	z12.rotation = 0.5
	game.timevent_add( bye_bye, ( z12, triggerer ), 30300 )
	z13 = game.obj_create(14663, location_from_axis(768, 437))
	z13.rotation = 1.5
	game.timevent_add( bye_bye, ( z13, triggerer ), 30300 )
	z14 = game.obj_create(14663, location_from_axis(768, 441))
	z14.rotation = 2.5
	game.timevent_add( bye_bye, ( z14, triggerer ), 30300 )
	z15 = game.obj_create(14663, location_from_axis(768, 445))
	z15.rotation = 3.5
	game.timevent_add( bye_bye, ( z15, triggerer ), 30300 )
	z16 = game.obj_create(14662, location_from_axis(768, 449))
	z16.rotation = 4.5
	game.timevent_add( bye_bye, ( z16, triggerer ), 30300 )
	z17 = game.obj_create(14663, location_from_axis(768, 453))
	z17.rotation = 5.5
	game.timevent_add( bye_bye, ( z17, triggerer ), 30300 )
	z18 = game.obj_create(14663, location_from_axis(772, 437))
	z18.rotation = 0.5
	game.timevent_add( bye_bye, ( z18, triggerer ), 30300 )
	z19 = game.obj_create(14663, location_from_axis(772, 441))
	z19.rotation = 1.5
	game.timevent_add( bye_bye, ( z19, triggerer ), 30300 )
	z20 = game.obj_create(14663, location_from_axis(688, 445))
	z20.rotation = 2.5
	game.timevent_add( bye_bye, ( z20, triggerer ), 30300 )
	z21 = game.obj_create(14662, location_from_axis(684, 445))
	z21.rotation = 3.5
	game.timevent_add( bye_bye, ( z21, triggerer ), 30300 )
	z22 = game.obj_create(14663, location_from_axis(684, 449))
	z22.rotation = 4.5
	game.timevent_add( bye_bye, ( z22, triggerer ), 30300 )
	z23 = game.obj_create(14663, location_from_axis(684, 453))
	z23.rotation = 5.5
	game.timevent_add( bye_bye, ( z23, triggerer ), 30300 )
	z24 = game.obj_create(14663, location_from_axis(684, 457))
	z24.rotation = 0.5
	game.timevent_add( bye_bye, ( z24, triggerer ), 30300 )
	z25 = game.obj_create(14663, location_from_axis(692, 465))
	z25.rotation = 1.5
	game.timevent_add( bye_bye, ( z25, triggerer ), 30300 )
	z26 = game.obj_create(14662, location_from_axis(692, 469))
	z26.rotation = 2.5
	game.timevent_add( bye_bye, ( z26, triggerer ), 30300 )
	z27 = game.obj_create(14663, location_from_axis(692, 473))
	z27.rotation = 3.5
	game.timevent_add( bye_bye, ( z27, triggerer ), 30300 )
	z28 = game.obj_create(14663, location_from_axis(688, 473))
	z28.rotation = 4.5
	game.timevent_add( bye_bye, ( z28, triggerer ), 30300 )
	z29 = game.obj_create(14663, location_from_axis(688, 457))
	z29.rotation = 5.5
	game.timevent_add( bye_bye, ( z29, triggerer ), 30300 )
	z30 = game.obj_create(14663, location_from_axis(680, 445))
	z30.rotation = 0.5
	game.timevent_add( bye_bye, ( z30, triggerer ), 30300 )
	game.sound( 4115, 1 )
	game.global_vars[696] = 9
	game.global_flags[869] = 0
	game.quests[83].state = qs_completed
	return RUN_DEFAULT


def bye_bye( attachee, triggerer ):
	attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT