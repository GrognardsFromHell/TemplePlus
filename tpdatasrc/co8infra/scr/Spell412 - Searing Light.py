from toee import *

def OnBeginSpellCast( spell ):
	print "Searing Light OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Searing Light OnSpellEffect"

def OnBeginRound( spell ):
	print "Searing Light OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Searing Light OnBeginProjectile"

	#spell.proj_partsys_id = game.particles( 'sp-Searing Light', projectile )
	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Searing Light', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Searing Light OnEndProjectile"


	damage_dice = dice_new( '1d8' )
	damage_dice.number = min( 5, spell.caster_level / 2 )

	spell.duration = 0

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )
	target_item = spell.target_list[0]

	npc = spell.caster
	if npc.name == 20003:
		for obj in game.party[0].group_list():
			if obj.name == 8072 and obj.leader_get() != OBJ_HANDLE_NULL:
				curr = obj.stat_level_get( stat_hp_current )
				if curr >= -9: 
					target_item.obj = obj

	####################################################
	# WF Ray fix added by Shiningted (& two lines below)
	####################################################

	has_it = 0
	x = 0
	y = 0

	if spell.caster.has_feat(feat_weapon_focus_ray):
		# game.particles( "sp-summon monster I", game.party[0] )
		has_it = 1
		x = spell.caster.stat_base_get(stat_dexterity)
		y = x + 2
		if spell.caster.has_feat(feat_greater_weapon_focus_ray):
			y = y + 2
		spell.caster.stat_base_set(stat_dexterity, y)

	####################################################

	################################################################################################
	# adding fix for an undead creature particularly vulnerable to bright light (& six lines below)
	################################################################################################

	# check if undead has a vulnerability to sunlight, an aversion to daylight, or a sunlight or daylight powerlessness
	# current creatures: Bodak, Nightwalker
	undead_list = [14328, 14958]
	if (target_item.obj.is_category_type( mc_type_undead )):
		undead_vulnerable = 0
		for undead in undead_list:
			if undead == target_item.obj.name:
				undead_vulnerable = 1

	################################################################################################

	attack_successful = spell.caster.perform_touch_attack( target_item.obj )

	if attack_successful & D20CAF_HIT:
		if index_of_target > 0:
			attack_successful |= D20CAF_NO_PRECISION_DAMAGE
		game.particles( 'sp-Searing Light-Hit', target_item.obj )

		# hit
		if (target_item.obj.is_category_type( mc_type_undead )):
			if undead_vulnerable == 1:
				damage_dice.size = 8
			else:
				damage_dice.size = 6
			damage_dice.number = min( 10, spell.caster_level )
			target_item.obj.spell_damage_weaponlike( spell.caster, D20DT_POSITIVE_ENERGY, damage_dice, D20DAP_UNSPECIFIED, 100, D20A_CAST_SPELL, spell.id , attack_successful, index_of_target)
		else:
			if (target_item.obj.is_category_type( mc_type_construct )):
				damage_dice.size = 6
			target_item.obj.spell_damage_weaponlike( spell.caster, D20DT_POSITIVE_ENERGY, damage_dice, D20DAP_UNSPECIFIED, 100, D20A_CAST_SPELL, spell.id , attack_successful, index_of_target)

	else:

		# missed
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )

		game.particles( 'Fizzle', target_item.obj )

	if has_it == 1:
		spell.caster.stat_base_set(stat_dexterity, x)

	spell.target_list.remove_target( target_item.obj )
	spell.spell_end( spell.id )

def OnEndSpellCast( spell ):
	print "Searing Light OnEndSpellCast"