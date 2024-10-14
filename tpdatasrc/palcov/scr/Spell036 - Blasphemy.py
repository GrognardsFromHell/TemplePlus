from toee import *
from utilities import dbug

def OnBeginSpellCast( spell ):
	print "Blasphemy OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def OnSpellEffect ( spell ):
	print "Blasphemy OnSpellEffect"

	remove_list = []
	spell.dc = spell.dc + 4  # will save vs. Banishment is -4 
	spell.caster_level = fix_caster_level(spell)
	
	# List can mutate during processing, so loop through a copy, marc
	for target_item in list(spell.target_list):  # for target_item in spell.target_list: 

		obj_hit_dice = target_item.obj.hit_dice_num
		alignment = target_item.obj.critter_get_alignment()
		dbug("  target_item.obj",target_item.obj,'holy')

		# Only works on non-evil creatures
		if not (alignment & ALIGNMENT_EVIL) and not (target_item.obj == spell.caster):

			game.particles( 'sp-Slay Living', target_item.obj )
			dbug("    target_item NOT EVIL",-99,'holy')

			# Anything ten or more levels below the caster's level dies
			if obj_hit_dice <= (spell.caster_level - 10):
				# So you'll get awarded XP for the kill
				if not target_item.obj in game.leader.group_list():
					target_item.obj.damage( game.leader , D20DT_UNSPECIFIED, dice_new( "1d1" ) )
				target_item.obj.critter_kill()

			# Anything five or more levels below the caster's level is paralyzed
			if obj_hit_dice <= (spell.caster_level - 5):
				spell.duration = game.random_range(1,10) * 10
				target_item.obj.condition_add_with_args( 'sp-Hold Monster', spell.id, spell.duration, 0 )

			# Anything one or more levels below the caster's level is weakened
			if obj_hit_dice <= (spell.caster_level - 1): 
				spell.duration = game.random_range(1,4) + game.random_range(1,4)
				dam_amount = game.random_range(1,6) + game.random_range(1,6)
				target_item.obj.condition_add_with_args( 'sp-Ray of Enfeeblement', spell.id, spell.duration, dam_amount )

			# Anything the caster's level or below is dazed
			if obj_hit_dice <= (spell.caster_level):
				spell.duration = 1
				target_item.obj.condition_add_with_args( 'sp-Daze', spell.id, spell.duration, 0 )

				# Summoned and extraplanar creatures below the caster's level are also banished
				# if they fail a Will save at -4
				if target_item.obj.d20_query_has_spell_condition( sp_Summoned ) or target_item.obj.npc_flags_get() & ONF_EXTRAPLANAR != 0:

					# allow Will saving throw to negate
					if target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
	
						# saving throw successful
						target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
						game.particles( 'Fizzle', target_item.obj )

					else:

						# saving throw unsuccessful
						target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )
	
						# creature is sent back to its own plane
						# kill for now
						target_item.obj.critter_kill()

		remove_list.append( target_item.obj )

	spell.target_list.remove_list( remove_list )
	spell.spell_end(spell.id)

def OnBeginRound( spell ):
	print "Blasphemy OnBeginRound"

def OnEndSpellCast( spell ):
	print "Blasphemy OnEndSpellCast"
	
#------------------------------------------------------------------------------
# Fix for cleric domain bug, which wrongly increases caster level by +1
# for each unfriendly target on the list.
#------------------------------------------------------------------------------
def fix_caster_level (spell):	
	dbug("\n\nBLASPHEMY\n----------------",-99,'holy')
	dbug("BEFORE  spell.caster_level",spell.caster_level,'holy')
	dbug("BEFORE  len(spell.target_list)",len(spell.target_list),'holy')	
	if spell.caster.obj_get_int(obj_f_critter_domain_1) == evil or spell.caster.obj_get_int(obj_f_critter_domain_2) == evil:
		if spell.caster.stat_level_get(stat_level_cleric) >= 13:
			for t in spell.target_list:
				if not spell.caster.is_friendly(t.obj):
					spell.caster_level = spell.caster_level - 1
	dbug("AFTER  spell.caster_level",spell.caster_level,'holy')
	return spell.caster_level
