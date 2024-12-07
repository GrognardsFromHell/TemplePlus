from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Modify Memory OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-enchantment-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Modify Memory OnSpellEffect"

	spell.duration = 1

	################################
	#
	# First find the nearest NPC to the target location
	#
	################################

	new_targ = 0
	dist = 4
	
	for obj in game.obj_list_vicinity(spell.target_loc, OLC_NPC):
		NEWdistance = 0
		if (obj.leader_get() == OBJ_HANDLE_NULL):
			x1, y1 = location_to_axis(obj.location)
			x2, y2 = location_to_axis(spell.target_loc)
			if x1 > x2:
				x3 = x1 - x2
			else:
				x3 = x2 - x1

			if y1 > y2:
				y3 = y1 - y2
			else:
				y3 = y2 - y1
			NEWdistance = ((x3*x3) + (y3*y3))**0.5
			bet = NEWdistance

		else:
			NEWdistance = 6
		if NEWdistance <= dist:
			dist = NEWdistance
			new_targ = obj

	if new_targ == OBJ_HANDLE_NULL or new_targ == 0:
		bob = spell.caster
		game.particles( 'Fizzle', bob )
		bob.float_mesfile_line( 'mes\\narrative.mes', 161 )
	else:
		game.particles( 'sp-Feat of Strength-END', new_targ)


	if new_targ.stat_level_get(stat_intelligence) >= 1:
		if not (new_targ.is_category_type( mc_type_undead ) or new_targ.is_category_type( mc_type_ooze ) or new_targ.is_category_type( mc_type_aberration ) or new_targ.is_category_type( mc_type_outsider ) or new_targ.is_category_type( mc_type_construct )):

			if new_targ.is_friendly( spell.caster ):
				if not new_targ.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):

					new_targ.reaction_adj( game.party[0], 40 )
					new_targ.npc_flag_unset(ONF_KOS)
					new_targ.npc_flag_set(ONF_KOS_OVERRIDE)

					x = new_targ.scripts[19]
					new_targ.scripts[19] = 0
					game.timeevent_add( reset_sid, ( new_targ, x ), 300000 )				
					game.particles( 'sp-Feat of Strength-END', new_targ )

					cozen = game.obj_create( 12696, new_targ.location)
					new_targ.item_get(cozen)
					cozen.object_script_execute( cozen, 28 )
					# game.particles( "sp-summon monster I", game.party[0] )

				else:

					# saving throw successful
					new_targ.float_mesfile_line( 'mes\\spell.mes', 30001 )

					vigilance = game.obj_create( 12695, new_targ.location)
					new_targ.item_get(vigilance)
					vigilance.object_script_execute( vigilance, 28 )
					# game.particles( "sp-summon monster I", game.party[0] )

			else:
				if not new_targ.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):

					new_targ.reaction_adj( game.party[0], 40 )
					new_targ.npc_flag_unset(ONF_KOS)
					new_targ.npc_flag_set(ONF_KOS_OVERRIDE)
					x = new_targ.scripts[19]
					new_targ.scripts[19] = 0
					game.timeevent_add( reset_sid, ( new_targ, x ), 300000 )
					game.particles( 'sp-Feat of Strength-END', new_targ )

					cozen = game.obj_create( 12696, new_targ.location)
					new_targ.item_get(cozen)
					cozen.object_script_execute( cozen, 28 )
					# game.particles( "sp-summon monster I", game.party[0] )

				else:

					# saving throw successful
					new_targ.float_mesfile_line( 'mes\\spell.mes', 30001 )

					vigilance = game.obj_create( 12695, new_targ.location)
					new_targ.item_get(vigilance)
					vigilance.object_script_execute( vigilance, 28 )
					# game.particles( "sp-summon monster I", game.party[0] )

		else:
			# something weird: undead, construct etc
			new_targ.float_mesfile_line( 'mes\\spell.mes', 30003 )
			game.particles( 'Fizzle', new_targ )

	else:

		# critters with no intelligence have no memories!
		new_targ.float_mesfile_line( 'mes\\spell.mes', 20055 )
		game.particles( 'Fizzle', new_targ )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Modify Memory OnBeginRound"

def OnEndSpellCast( spell ):
	print "Modify Memory OnEndSpellCast"

def reset_sid(targ, id_x):
	targ.scripts[19] = id_x
	game.sound(7461,1)
	game.particles( 'Fizzle', targ )
	return