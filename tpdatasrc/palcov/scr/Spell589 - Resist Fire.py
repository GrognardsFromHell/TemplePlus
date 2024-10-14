from toee import *
from co8Util.spells import *

def OnBeginSpellCast( spell ):
	print "Resist Fire OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Resist Fire OnSpellEffect"

	element_type = FIRE
	partsys_type = 'sp-Resist Elements-fire'
	pd_key = RESIST_FIRE_KEY

	spell.duration = spell.caster_level*100
	if (spell.caster not in game.party) and (spell.caster.object_flags_get() & OF_INVULNERABLE == 0):
		spell.duration = 600*24*30  # 30 days

	target_item = spell.target_list[0]
	
	# Archer's Brew, 'Resist Fire (INTERNAL)' domain_special 11
	if spell.caster_level == 21 or (target_item.obj not in game.party and spell.caster.name != 14467):  # pool caster
		spell.caster_level = 15
		spell.duration = 100
		dex_amount = 4
		num_of_images = 8
		target_item.obj.condition_add_with_args( 'sp-Cats Grace', spell.id, spell.duration, dex_amount )
		target_item.obj.condition_add_with_args( 'sp-Haste', spell.id, spell.duration, 1 )
		target_item.obj.condition_add_with_args( 'sp-Mirror Image', spell.id, spell.duration, num_of_images )
		target_item.obj.condition_add_with_args( 'sp-Resist Elements', spell.id, COLD, spell.duration )

	if target_item.obj.condition_add_with_args( 'sp-Resist Elements', spell.id, element_type, spell.duration ):
		add_to_persistent_list (pd_key, spell.id, target_item.obj)  # marc
		target_item.partsys_id = game.particles( partsys_type, target_item.obj )

	# Pool
	if spell.caster.name == 14467:
		spell.caster_level = game.global_vars[911]
		for target in game.party:
			if target.condition_add_with_args( 'sp-Resist Elements', spell.id, element_type, spell.duration ):
				add_to_persistent_list (pd_key, spell.id, target)  # marc
				game.particles( 'sp-dispel air-end', target )
				game.particles( 'sp-aid-end', target )
		game.particles_end(target_item.partsys_id)  # so npc caster doesn't get particles 
		
def OnBeginRound( spell ):
	print "Resist Fire OnBeginRound"

def OnEndSpellCast( spell ):
	print "Resist Fire OnEndSpellCast"
	remove_from_persistent_list (RESIST_FIRE_KEY, spell.id)  # marc
