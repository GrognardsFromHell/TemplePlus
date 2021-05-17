from toee import *
from utilities import *


def san_dialog( attachee, triggerer ):
	triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT
	
def san_insert_item( attachee, triggerer ):	## this is for oil, acid, Alchemist's fire etc
	return RUN_DEFAULT # this script was buggy; but it is no longer needed, root bug causing grenades to appear on the corpse is now fixed
	print("san_insert_item: attachee = ", attachee, ' triggerer = ', triggerer, triggerer.name,' cap1, cap2 = ', cap1, cap2)
	cap1 = triggerer.name
	
	cap2 = attachee.obj_get_int(obj_f_item_pad_i_1)

	
	## first check if the user is just moving it around internally (eg equipping it)
	
	if cap2 == cap1:
		return RUN_DEFAULT
	## next check if there was no previous user (getting from a box or off ground)
	elif cap2 == 0:
		if (triggerer.type == obj_t_pc or triggerer.type == obj_t_npc):
			attachee.obj_set_int( obj_f_item_pad_i_1, cap1 )
		return RUN_DEFAULT
	## next check if it is being inserted INTO box or dropped onto ground
	## hmmm... this doesn't work, PC remains triggerer... I wonder...
	elif (triggerer.type != obj_t_pc and triggerer.type != obj_t_npc):
		# game.particles( "sp-summon monster I", game.party[0] )
		attachee.obj_set_int( obj_f_item_pad_i_1, 0 )
		return RUN_DEFAULT
	## now the money shot
	## if this takes place in combat, then it's going from one combatant to another
	## add splash effect object
	elif game.combat_is_active():
		cap3 = attachee.name
		attachee.destroy()
		## next section only for KotB, requires certain strategy changes
		## and deals with items like Alchemist's Fire not in ToEE
		# if cap3 == 4643 or cap3 == 4644 or cap3 == 4645:
		#	splash_effect = game.obj_create(12833, triggerer.location)
		#	triggerer.item_get(splash_effect)
		#	splash_effect.obj_set_int( obj_f_item_pad_i_1, cap3 )
		#	## set original weapon name on residue items
		#	## so residue effect can be individually scripted
		#	game.timeevent_add( get_rid_of_it, ( splash_effect, triggerer ), 1500 )
	## outside combat we don't have to worry, it's just party members moving it around
	return RUN_DEFAULT

def san_remove_item( attachee, triggerer ):
	attachee.destroy()
	return RUN_DEFAULT

def get_rid_of_it(obj, victim):
	eff1 = obj.obj_get_int(obj_f_item_pad_i_1)
	if eff1 == 0 or eff1 == OBJ_HANDLE_NULL:
		return
	eff2 = obj.name
	game.global_vars[902] = obj.obj_get_int(obj_f_item_pad_i_1)
	if eff1 == 4643:	## alchemist's fire
		dam = dice_new( '1d6' )
		dtype = D20DT_FIRE
		game.particles( "hit-FIRE-medium", victim )
	elif eff1 == 4644:	## alchemist's spark
		dam = dice_new( '1d8' )
		dtype = D20DT_ELECTRICITY
		game.particles( "hit-SHOCK-medium", victim )
	elif eff1 == 4645:	## alchemist's frost
		dam = dice_new( '1d8' )
		dtype = D20DT_COLD
		game.particles( "hit-COLD-Burst", victim )
	if dam > 0:
		victim.damage( OBJ_HANDLE_NULL, dtype, dam )
		victim.float_mesfile_line( 'mes\\combat.mes', 6500 )
	obj.destroy()
	return