from toee import *
from utilities import *
from py00439script_daemon import within_rect_by_corners
from combat_standard_routines import *


def san_dying( attachee, triggerer ):
	# if should_modify_CR( attachee ): # no longer necessary in Temple+
		# modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if (attachee.name == 8960):
		if (game.global_flags[554] == 0):
			game.global_flags[553] = 1
	elif (attachee.name == 8966):
		if (game.global_flags[553] == 0):
			game.global_flags[554] = 1
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	# webbed = break_free( attachee, 3) # no longer necessaryin Temple+
############################################
#  ready vs approach guys with explosives  #
############################################
	if (attachee.name == 8932 or attachee.name == 8936 or attachee.name == 8940 or attachee.name == 8944 or attachee.name == 8948 or attachee.name == 8952 or attachee.name == 8956):
		if (obj_percent_hp(attachee) <= 50):
			attachee.obj_set_int(obj_f_critter_strategy, 547)
		elif (obj_percent_hp(attachee) >= 51):
			attachee.obj_set_int(obj_f_critter_strategy, 565)
			for obj in game.party:
				if (within_rect_by_corners(obj, 423, 453, 488, 554)):
					damage_dice = dice_new( '6d6' )
					game.particles( "hit-BLUDGEONING-medium", obj )
					game.particles( "hit-FIRE-burst", obj )
					obj.float_mesfile_line( 'mes\\float.mes', 1 )
					if (obj.reflex_save_and_damage( OBJ_HANDLE_NULL, 20, D20_Save_Reduction_Half, D20STD_F_NONE, damage_dice, D20DT_FORCE, D20DAP_UNSPECIFIED, 0 , D20DAP_NORMAL )):
						obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
					else:
						obj.float_mesfile_line( 'mes\\spell.mes', 30002 )
					game.global_flags[872] = 1
			if (game.global_flags[872] == 1):
				go_boom_one_time( attachee, triggerer )
###################################
#  ready vs approach guys normal  #
###################################
	elif (attachee.name == 8931 or attachee.name == 8933 or attachee.name == 8934 or attachee.name == 8935 or attachee.name == 8937 or attachee.name == 8938 or attachee.name == 8939 or attachee.name == 8946 or attachee.name == 8941 or attachee.name == 8942 or attachee.name == 8943 or attachee.name == 8945 or attachee.name == 8946 or attachee.name == 8947 or attachee.name == 8949 or attachee.name == 8950 or attachee.name == 8951 or attachee.name == 8953 or attachee.name == 8954 or attachee.name == 8955 or attachee.name == 8967 or attachee.name == 8958 or attachee.name == 8969 or attachee.name == 8970 or attachee.name == 8971 or attachee.name == 8972 or attachee.name == 8973 or attachee.name == 8974 or attachee.name == 8975 or attachee.name == 8976 or attachee.name == 8977):
		if (obj_percent_hp(attachee) <= 50):
			attachee.obj_set_int(obj_f_critter_strategy, 547)
		elif (obj_percent_hp(attachee) >= 51):
			attachee.obj_set_int(obj_f_critter_strategy, 565)
###################
#  RANGED TROOPS  #
############################
#  dumb guys - orc bowmen  #
############################
	elif (attachee.name == 8978 or attachee.name == 8986 or attachee.name == 8989):
		leader = attachee.leader_get()
		if (group_percent_hp(leader) >= 51):
			attachee.obj_set_int(obj_f_critter_strategy, 552)
		elif (group_percent_hp(leader) <= 50):
			attachee.obj_set_int(obj_f_critter_strategy, 553)
##############################
#  smart guys - orc archers  #
##############################
	elif (attachee.name == 8979 or attachee.name == 8983 or attachee.name == 8984 or attachee.name == 8985 or attachee.name == 8991):
		leader = attachee.leader_get()
		if (group_percent_hp(leader) >= 51):
			attachee.obj_set_int(obj_f_critter_strategy, 551)
		elif (group_percent_hp(leader) <= 50):
			attachee.obj_set_int(obj_f_critter_strategy, 553)
################################
#  mage seekers - orc snipers  #
################################
	elif (attachee.name == 8980 or attachee.name == 8982 or attachee.name == 8987 or attachee.name == 8990):
		leader = attachee.leader_get()
		if (group_percent_hp(leader) >= 51):
			attachee.obj_set_int(obj_f_critter_strategy, 555)
		elif (group_percent_hp(leader) <= 50):
			attachee.obj_set_int(obj_f_critter_strategy, 553)
##################################
#  archery seekers - nobody atm  #
##################################
#	elif (attachee.name == xxxxx):
#		leader = attachee.leader_get()
#		if (group_percent_hp(leader) >= 51):
#			attachee.obj_set_int(obj_f_critter_strategy, 554)
#		elif (group_percent_hp(leader) <= 50):
#			attachee.obj_set_int(obj_f_critter_strategy, 553)
#####################################
#  spell responders - orc marksmen  #
#####################################
	elif (attachee.name == 8981 or attachee.name == 8988):
		for obj in pc.group_list():
			if (obj.stat_level_get(stat_level_wizard) >= 1 or obj.stat_level_get(stat_level_sorcerer) >= 1 or obj.stat_level_get(stat_level_druid) >= 1 or obj.stat_level_get(stat_level_bard) >= 1):
				leader = attachee.leader_get()
				if (group_percent_hp(leader) >= 34):
					attachee.obj_set_int(obj_f_critter_strategy, 557)
				elif (group_percent_hp(leader) <= 33):
					attachee.obj_set_int(obj_f_critter_strategy, 553)
			else:
				leader = attachee.leader_get()
				if (group_percent_hp(leader) >= 34):
					attachee.obj_set_int(obj_f_critter_strategy, 554)
				elif (group_percent_hp(leader) <= 33):
					attachee.obj_set_int(obj_f_critter_strategy, 553)
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (attachee.name == 8931):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 574 )
			attachee.standpoint_set( STANDPOINT_DAY, 574 )
			game.new_sid = 0
	elif (attachee.name == 8932):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 575 )
			attachee.standpoint_set( STANDPOINT_DAY, 575 )
			game.new_sid = 0
	elif (attachee.name == 8933):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 576 )
			attachee.standpoint_set( STANDPOINT_DAY, 576 )
			game.new_sid = 0
	elif (attachee.name == 8934):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 579 )
			attachee.standpoint_set( STANDPOINT_DAY, 579 )
			game.new_sid = 0
	elif (attachee.name == 8935):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 580 )
			attachee.standpoint_set( STANDPOINT_DAY, 580 )
			game.new_sid = 0
	elif (attachee.name == 8936):
		if (game.global_flags[554] == 1):
			attachee.unconceal()
			attachee.standpoint_set( STANDPOINT_NIGHT, 581 )
			attachee.standpoint_set( STANDPOINT_DAY, 581 )
			game.new_sid = 0
	elif (attachee.name == 8937):
		if (game.global_flags[554] == 1):
			attachee.unconceal()
			attachee.standpoint_set( STANDPOINT_NIGHT, 582 )
			attachee.standpoint_set( STANDPOINT_DAY, 582 )
			game.new_sid = 0
	elif (attachee.name == 8938):
		if (game.global_flags[554] == 1):
			attachee.unconceal()
			attachee.standpoint_set( STANDPOINT_NIGHT, 583 )
			attachee.standpoint_set( STANDPOINT_DAY, 583 )
			game.new_sid = 0
	elif (attachee.name == 8939):
		if (game.global_flags[554] == 1):
			attachee.unconceal()
			attachee.standpoint_set( STANDPOINT_NIGHT, 584 )
			attachee.standpoint_set( STANDPOINT_DAY, 584 )
			game.new_sid = 0
	elif (attachee.name == 8969):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 585 )
			attachee.standpoint_set( STANDPOINT_DAY, 585 )
			game.new_sid = 0
	elif (attachee.name == 8970):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 586 )
			attachee.standpoint_set( STANDPOINT_DAY, 586 )
			game.new_sid = 0
	elif (attachee.name == 8971):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 587 )
			attachee.standpoint_set( STANDPOINT_DAY, 587 )
			game.new_sid = 0
	elif (attachee.name == 8959):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 588 )
			attachee.standpoint_set( STANDPOINT_DAY, 588 )
			game.new_sid = 0
	elif (attachee.name == 8960):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 589 )
			attachee.standpoint_set( STANDPOINT_DAY, 589 )
			game.new_sid = 0
	elif (attachee.name == 8961):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 590 )
			attachee.standpoint_set( STANDPOINT_DAY, 590 )
			game.new_sid = 0
	elif (attachee.name == 8978):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 591 )
			attachee.standpoint_set( STANDPOINT_DAY, 591 )
			game.new_sid = 0
	elif (attachee.name == 8979):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 592 )
			attachee.standpoint_set( STANDPOINT_DAY, 592 )
			game.new_sid = 0
	elif (attachee.name == 8980):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 593 )
			attachee.standpoint_set( STANDPOINT_DAY, 593 )
			game.new_sid = 0
	elif (attachee.name == 8981):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 594 )
			attachee.standpoint_set( STANDPOINT_DAY, 594 )
			game.new_sid = 0
	elif (attachee.name == 8982):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 595 )
			attachee.standpoint_set( STANDPOINT_DAY, 595 )
			game.new_sid = 0
	elif (attachee.name == 8983):
		if (game.global_flags[554] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 596 )
			attachee.standpoint_set( STANDPOINT_DAY, 596 )
			game.new_sid = 0
	elif (attachee.name == 8950):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 597 )
			attachee.standpoint_set( STANDPOINT_DAY, 597 )
			game.new_sid = 0
	elif (attachee.name == 8951):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 598 )
			attachee.standpoint_set( STANDPOINT_DAY, 598 )
			game.new_sid = 0
	elif (attachee.name == 8952):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 599 )
			attachee.standpoint_set( STANDPOINT_DAY, 599 )
			game.new_sid = 0
	elif (attachee.name == 8953):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 600 )
			attachee.standpoint_set( STANDPOINT_DAY, 600 )
			game.new_sid = 0
	elif (attachee.name == 8954):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 695 )
			attachee.standpoint_set( STANDPOINT_DAY, 695 )
			game.new_sid = 0
	elif (attachee.name == 8955):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 696 )
			attachee.standpoint_set( STANDPOINT_DAY, 696 )
			game.new_sid = 0
	elif (attachee.name == 8956):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 697 )
			attachee.standpoint_set( STANDPOINT_DAY, 697 )
			game.new_sid = 0
	elif (attachee.name == 8957):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 698 )
			attachee.standpoint_set( STANDPOINT_DAY, 698 )
			game.new_sid = 0
	elif (attachee.name == 8958):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 699 )
			attachee.standpoint_set( STANDPOINT_DAY, 699 )
			game.new_sid = 0
	elif (attachee.name == 8975):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 183 )
			attachee.standpoint_set( STANDPOINT_DAY, 183 )
			game.new_sid = 0
	elif (attachee.name == 8976):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 184 )
			attachee.standpoint_set( STANDPOINT_DAY, 184 )
			game.new_sid = 0
	elif (attachee.name == 8977):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 887 )
			attachee.standpoint_set( STANDPOINT_DAY, 887 )
			game.new_sid = 0
	elif (attachee.name == 8965):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 890 )
			attachee.standpoint_set( STANDPOINT_DAY, 890 )
			game.new_sid = 0
	elif (attachee.name == 8966):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 292 )
			attachee.standpoint_set( STANDPOINT_DAY, 292 )
			game.new_sid = 0
	elif (attachee.name == 8967):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 888 )
			attachee.standpoint_set( STANDPOINT_DAY, 888 )
			game.new_sid = 0
	elif (attachee.name == 8968):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 889 )
			attachee.standpoint_set( STANDPOINT_DAY, 889 )
			game.new_sid = 0
	elif (attachee.name == 8985):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 293 )
			attachee.standpoint_set( STANDPOINT_DAY, 293 )
			game.new_sid = 0
	elif (attachee.name == 8987):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 294 )
			attachee.standpoint_set( STANDPOINT_DAY, 294 )
			game.new_sid = 0
	elif (attachee.name == 8989):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 295 )
			attachee.standpoint_set( STANDPOINT_DAY, 295 )
			game.new_sid = 0
	elif (attachee.name == 8991):
		if (game.global_flags[553] == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 296 )
			attachee.standpoint_set( STANDPOINT_DAY, 296 )
			game.new_sid = 0
	return RUN_DEFAULT


def go_boom_one_time( attachee, triggerer ):
	game.sound( 4170, 1 )
	game.global_flags[872] = 0
	return