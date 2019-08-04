from toee import *
from utilities import *
from combat_standard_routines import *


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	game.particles( 'DesecrateEarth', attachee )
	for obj in game.party[0].group_list():
		if obj.distance_to(attachee) <= 60 or attachee.has_los(obj):
			if not obj.d20_query_has_spell_condition( sp_Protection_From_Alignment ) and not obj.is_category_type( mc_type_undead ):
				alignment = obj.critter_get_alignment()
				if (alignment & ALIGNMENT_EVIL):
					game.particles( 'Barbarian Rage-end', obj )
					create_item_in_inventory(12752, obj)
					create_item_in_inventory(12752, obj)
					obj.float_mesfile_line( 'mes\\combat.mes', 6016 )
				elif (alignment & ALIGNMENT_GOOD):
					game.particles( 'Barbarian Rage-end', obj )
					create_item_in_inventory(12750, obj)
					create_item_in_inventory(12750, obj)
					obj.float_mesfile_line( 'mes\\combat.mes', 6016 )
				elif (alignment & ALIGNMENT_LAWFUL):
					game.particles( 'Barbarian Rage-end', obj )
					create_item_in_inventory(12749, obj)
					create_item_in_inventory(12749, obj)
					obj.float_mesfile_line( 'mes\\combat.mes', 6016 )
				elif (alignment & ALIGNMENT_CHAOTIC):
					game.particles( 'Barbarian Rage-end', obj )
					create_item_in_inventory(12751, obj)
					create_item_in_inventory(12751, obj)
					obj.float_mesfile_line( 'mes\\combat.mes', 6016 )
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	for blech in game.obj_list_vicinity(attachee.location, OLC_NPC):
		# game.particles( 'sp-Bless Water', blech )
		if (blech.distance_to(attachee) <= 60):
			# game.particles( 'DesecrateEarth', attachee )
			if blech.is_category_type( mc_type_undead ):
				dice = dice_new( "5d1" )
				blech.heal( attachee, dice )
				if blech.stat_level_get( stat_hp_current ) > 0:
					game.particles( 'sp-Curse Water', blech )
	return RUN_DEFAULT


def san_insert_item( attachee, triggerer ):
	game.timeevent_add( make_it_gone, ( attachee ), 20000 )
	return RUN_DEFAULT


def make_it_gone(attachee):
	attachee.destroy()
	# game.particles( "sp-summon monster I", game.party[0] )
	return