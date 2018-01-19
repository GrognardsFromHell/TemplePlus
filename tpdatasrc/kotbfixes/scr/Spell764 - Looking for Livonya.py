from toee import *

from utilities import *

def OnBeginSpellCast( spell ):
	print "Looking for Livonya OnBeginSpellCast"

def OnSpellEffect( spell ):
	print "Looking for Livonya OnSpellEffect"

	usr = spell.caster
	found = 0

	if usr.map == 5002 or usr.map == 5094:		## Southern Pine Woods, ## Northern Oak Woods
		DC = 23
	elif usr.map == 5060 or usr.map == 5011 or usr.map == 5061:	## Gnolls, Guildhouse
		DC = 30
	elif usr.map == 5016:	## Fortress Basement
		DC = 26
	elif usr.map == 5034 or usr.map == 5058 or usr.map == 5059:	## Pub, Bugbear, Labyrinth
		DC = 28
	elif usr.map == 5123 or usr.map == 5056:	## Hobgobs, Shopmap
		DC = 23
	elif usr.map == 5053: # Orc Caves
		DC = 23
	else:
		DC = 25

	## v 1.0: alignment restrictions

	if usr.map == 5123 and (game.party_alignment != NEUTRAL_GOOD and game.party_alignment != NEUTRAL_EVIL and game.party_alignment != LAWFUL_NEUTRAL):
		for obj in game.obj_list_vicinity(usr.location,OLC_SCENERY):
			if ((obj.name == 2036 or obj.name == 2037) and usr.distance_to(obj) <= 35):
				for pc in game.party:
					level = pc.skill_level_get(skill_search)
					level = level + 20
					if (pc.d20_query_has_spell_condition( sp_Guidance ) != 0):
						level = level + 1
					if (level >= DC and pc.distance_to(obj) <= 35):
						pc.float_mesfile_line( 'mes\\skill_ui.mes', 1004, tf_red )


	## generic secret ladders 

	elif (usr.map == 5002 or usr.map == 5094 or usr.map == 5123 or usr.map == 5016 or usr.map == 5056):
		for obj in game.obj_list_vicinity(usr.location,OLC_SCENERY):
			if ((obj.name == 2036 or obj.name == 2037) and usr.distance_to(obj) <= 35):
				for pc in game.party:
					level = pc.skill_level_get(skill_search)
					level = level + 20
					if (pc.d20_query_has_spell_condition( sp_Guidance ) != 0):
						level = level + 1
					if (level >= DC and pc.distance_to(obj) <= 35):
						found = 1
						target = obj
						pc.float_mesfile_line( 'mes\\skill_ui.mes', 1002 )

	## hobgobs
	if (usr.map == 5054):
		for obj in game.obj_list_vicinity(usr.location,OLC_SCENERY):
			if ((obj.name == 2035 or obj.name == 2011) and usr.distance_to(obj) <= 35):
				for pc in game.party:
					DC = 25
					level = pc.skill_level_get(skill_search)
					level = level + 20
					if (level >= DC and pc.distance_to(obj) <= 35):
						found = 1
						target = obj
						pc.float_mesfile_line( 'mes\\skill_ui.mes', 1002 )
						if obj.name == 2035:
							game.global_flags[510] = 1

	## hobgob lever
	if (usr.map == 5056):
		for obj in game.obj_list_vicinity(usr.location,OLC_PORTAL):
			if ((obj.name == 69) and usr.distance_to(obj) <= 35):
				for pc in game.party:
					DC = 25
					level = pc.skill_level_get(skill_search)
					level = level + 20
					if (level >= DC and pc.distance_to(obj) <= 35):
						found = 1
						target = obj
						pc.float_mesfile_line( 'mes\\skill_ui.mes', 1003 )
				if found == 1:
					game.particles( "ef-MinoCloud", target )

	## generic secret doors
	if (usr.map == 5055 or usr.map == 5060 or usr.map == 5011 or usr.map == 5058 or usr.map == 5061 or usr.map == 5059 or usr.map == 5053):
		for obj in game.obj_list_vicinity(usr.location,OLC_SCENERY):
			if (obj.name == 2035 and usr.distance_to(obj) <= 25):
				for pc in game.party:
					level = pc.skill_level_get(skill_search)
					level = level + 20
					if (pc.d20_query_has_spell_condition( sp_Guidance ) != 0):
						level = level + 1
					if (level >= DC and pc.distance_to(obj) <= 25):
						found = 1
						target = obj
						pc.float_mesfile_line( 'mes\\skill_ui.mes', 1002 )
						if usr.map == 5053:
							target.object_flag_unset(OF_DONTDRAW) # Because there are two SD icons here

	## generic secret stairs
	if (usr.map == 5034):
		for obj in game.obj_list_vicinity(usr.location,OLC_SCENERY):
			if (obj.name == 2038 and usr.distance_to(obj) <= 35):
				for pc in game.party:
					level = pc.skill_level_get(skill_search)
					level = level + 20
					if (pc.d20_query_has_spell_condition( sp_Guidance ) != 0):
						level = level + 1
					if (level >= DC and pc.distance_to(obj) <= 35):
						found = 1
						target = obj
						pc.float_mesfile_line( 'mes\\skill_ui.mes', 1005 )

	if found == 1:
		target.object_flag_unset(OF_DONTDRAW)

	if found == 0:

		for pc in game.party:

			pc.float_mesfile_line( 'mes\\skill_ui.mes', 1300 )
	print "ending spell"
	spell.spell_end( spell.id, 1 )


def OnBeginRound( spell ):
	print "Looking for Livonya OnBeginRound"
	spell.spell_end( spell.id , 1 )

def OnEndSpellCast( spell ):
	print "Looking for Livonya OnEndSpellCast"
