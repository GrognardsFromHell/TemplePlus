from toee import *
import tpdp

def OnBeginSpellCast( spell ):
	print "Magic Missile OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

	#spell.num_of_projectiles = spell.num_of_projectiles + 1
	#spell.target_list.push_target(spell.caster)     # didn't work :(
	# generally the sequence is: OnBeginSpellCast, OnBeginProjectile, OnSpellEffect,OnEndProjectile (OnBeginRound isn't called)

def OnSpellEffect( spell ):
	print "Magic Missile OnSpellEffect"

	# only enable mirror image behavior in strict rules
	if not tpdp.config_get_bool('stricterRulesEnforcement'): return

	# Calculate which missiles hit mirror images instead of the actual target.
	#
	# This needs to be calculated ahead of time if we want to simulate picking
	# targets _before_ we know which is the real one, for situations where there
	# are more missiles than (real and fake) targets.
	offsets = []
	seen = []
	hits = [0,0,0,0,0]
	hi = -1
	for target_item in spell.target_list:
		target = target_item.obj
		hi += 1

		# handles can't be hashed, so we need to do something like this
		if target in seen:
			ix = seen.index(target)
			# if the same spell target occurs more than once, assume we're targeting
			# as many distinct copies if possible
			off, copies = offsets[ix]
			offsets[ix] = (off+1, copies)

			if off % copies > 0: hits[hi] = 1
		else:
			mirrors = target.d20_query(Q_Critter_Has_Mirror_Image)
			# no mirrors, always hit
			if mirrors <= 0: continue

			copies = mirrors+1
			off = dice_new(1, copies, -1).roll()

			# save the _next_ offset
			offsets.append((off+1, copies))
			seen.append(target)

			if off > 0: hits[hi] = 1

	h1, h2, h3, h4, h5 = hits
	spell.caster.condition_add_with_args('Magic Missile Mirror', spell.id, h1, h2, h3, h4, h5)

def OnBeginRound( spell ):
	print "Magic Missile OnBeginRound"


def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Magic Missile OnBeginProjectile"

	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-magic missle-proj', projectile ) )
	

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Magic Missile OnEndProjectile"

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )

	target_item = spell.target_list[ index_of_target ]
	target = target_item.obj

	hit_mirror = spell.caster.d20_query_with_data('Missile Mirror Hit', spell.id, index_of_target + 1)

	if hit_mirror > 0:
		if target.d20_query(Q_Critter_Has_Mirror_Image) > 0:
			mirror_id = target.d20_query_get_data(Q_Critter_Has_Mirror_Image, 0)
			target.d20_send_signal(S_Spell_Mirror_Image_Struck, mirror_id, 0)
			target.float_mesfile_line('mes\\combat.mes', 109)
			game.create_history_from_pattern(10, spell.caster, target)
	else: # normal damage
		damage_dice = dice_new(1,4,1)
		is_enemy = not spell.caster in game.party[0].group_list()
		target_charmed = target.d20_query(Q_Critter_Is_Charmed)
		if is_enemy and target_charmed:
			# NPC enemy is trying to cast on a charmed target - this is mostly meant for the Cult of the Siren encounter
			target = party_closest( spell.caster, conscious_only= 1, mode_select= 1, exclude_warded= 1, exclude_charmed = 1) # select nearest conscious PC instead, who isn't already charmed
			if target == OBJ_HANDLE_NULL:
				target = target_item.obj

		# always hits
		target.condition_add_with_args('sp-Magic Missile', spell.id, spell.duration, damage_dice.roll())
		target_item.partsys_id = game.particles('sp-magic missle-hit', target)

	# special scripting for NPCs no longer necessary - NPCs will launch multiple projectiles now

	#spell.target_list.remove_target_by_index( index_of_target )
	spell.num_of_projectiles -= 1

	if spell.num_of_projectiles == 0:
##		loc = target.location
##		target.destroy()
##		mxcr = game.obj_create( 12021, loc )
##		game.global_vars[30] = game.global_vars[30] + 1
		spell.caster.d20_send_signal(S_Spell_End, spell.id)
		spell.spell_end( spell.id, 1 )

def OnEndSpellCast( spell ):
	print "Magic Missile OnEndSpellCast"
