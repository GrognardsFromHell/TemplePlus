from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Magic Missile OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Magic Missile OnSpellEffect"

def OnBeginRound( spell ):
	print "Magic Missile OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Magic Missile OnBeginProjectile"
	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-magic missle-proj', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Magic Missile OnEndProjectile"

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )

	target = spell.target_list[ index_of_target ]

	damage_dice = dice_new( '1d4' )
	damage_dice.bonus = 1

	# Brooch of Shielding
	brooch = OBJ_HANDLE_NULL
	neck = target.obj.item_worn_at(1)
	if neck != OBJ_HANDLE_NULL and neck.name == 12652:
		brooch = neck
		extra = 0

	# first missile
	if brooch == OBJ_HANDLE_NULL:
		target.obj.condition_add_with_args( 'sp-Magic Missile', spell.id, spell.duration, damage_dice.roll() )
		target.partsys_id = game.particles( 'sp-magic missle-hit', target.obj )

	npc = spell.caster

	## The following section scripts extra damage dice for NPC casters
	## This is necessary because they can't shoot more than one bolt of Magic Missile (hardcoded engine limitation)
	## The workaround is to apply extra damage for that single bolt, custom tailored for each NPC caster

	if not spell.caster in game.party:

		# 1 extra missile - gnome wiz, hannah, lula, corona, goblin wiz, bug witch, wiz 3, wiz 4, researcher, 14874(monkey witch)
		if npc.name in [14073, 14515,14538,14560,14710,14711,14718,14719,14754,14874]:
			extra = 1

		# 2 extra missiles, 14503(brollo), 16604(jorrla), 14655(wexton), 14686(asher), 14785(vend councilor), wiz 5
		elif npc.name in [14503,14604,14655,14686,14785,14741]:
			extra = 2

		# 3 extra missiles, 14540(drowsila), 14670(tivanya), 14688(tiamara), 14690(burks), 14927(avoral), 14932(night hag), wiz 7
		elif npc.name in [14540,14670,14688,14690,14927,14932,14792]:
			extra = 3

		# 4 extra missiles, 14179(drow noble), 14268, 8300 (darley), 14333(vincent), 14499(drow witch),
		# 14537(thormund), 14557(zahara), 14652(liossa), 14586(sleepy), 14622(gil),
		# 14684(mancy), 14715(bug witch), 14766(evoker), 14767(illusionist),
		# 14893(orc warlock), 14968(lich), 14793(wiz 9), 14980(test wiz)
		elif npc.name in [14179,14268,8300,14333,14499,8121,8122,8123,14537,14557,14652,14586,14622,14684,14715,14766,14767,14893,14968,14793,14980]:
			extra = 4

		if brooch == OBJ_HANDLE_NULL:
			for x in range(0,extra):
				target.obj.condition_add_with_args( 'sp-Magic Missile', spell.id, spell.duration, damage_dice.roll() )
				target.partsys_id = game.particles( 'sp-magic missle-hit', target.obj )

	# Brooch of Shielding
	if brooch:
		target.obj.float_mesfile_line( 'mes\\spell.mes', 30019 )
		target.obj.float_mesfile_line( 'mes\\description.mes', 12652 )

		brooch_hp = brooch.obj_get_int(obj_f_secretdoor_dc)
		if brooch_hp == 0:  # first use, set to 101 hit points
			brooch_hp = 101

		# Estimate potential spell damage, assume 3.5 damage per missile
		brooch_hp -= int(3.5 * (1 + extra))

		# Subtract hit points from the brooch
		if brooch_hp > 0:
			brooch.obj_set_int(obj_f_secretdoor_dc,brooch_hp)
			float_num(target.obj,brooch_hp,2)
			target.partsys_id = game.particles( 'sp-shield-end', target.obj )
			game.sound(14522,1)

		# Brooch has no hit points left, destroy it
		else:
			brooch.destroy()
			target.obj.float_mesfile_line( 'mes\\combat.mes', 187 )
			target.partsys_id = game.particles( 'sp-spiritual weapon-end', target.obj )
			target.partsys_id = game.particles( 'sp-unholy water', target.obj )
			game.sound(15001,1)

	#spell.target_list.remove_target_by_index( index_of_target )
	spell.num_of_projectiles = spell.num_of_projectiles - 1

	if ( spell.num_of_projectiles == 0 ):
		spell.spell_end( spell.id, 1 )

def OnEndSpellCast( spell ):
	print "Magic Missile OnEndSpellCast"