from toee import *
from co8Util.spells import *

def OnBeginSpellCast( spell ):
	print "Resist Electricity OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Resist Electricity OnSpellEffect"

	element_type = ELECTRICITY
	partsys_type = 'sp-Resist Elements-electricity'
	pd_key = RESIST_ELECTRICITY_KEY
	
	spell.duration = spell.caster_level*100
	if (spell.caster not in game.party) and (spell.caster.object_flags_get() & OF_INVULNERABLE == 0):
		spell.duration = 600*24*30  # 30 days

	target_item = spell.target_list[0]
	
	if target_item.obj.condition_add_with_args( 'sp-Resist Elements', spell.id, element_type, spell.duration ):
		add_to_persistent_list (pd_key, spell.id, target_item.obj)  # marc
		target_item.partsys_id = game.particles( partsys_type, target_item.obj )

	if spell.caster.name == 14467:  # pool
		spell.caster_level = game.global_vars[911]
		for target in game.party:
			if target.condition_add_with_args( 'sp-Resist Elements', spell.id, element_type, spell.duration ):
				add_to_persistent_list (pd_key, spell.id, target)  # marc
				game.particles( 'sp-dispel air-end', target )
				game.particles( 'sp-aid-end', target )
		game.particles_end(target_item.partsys_id)  # so npc caster doesn't get particles 
		
def OnBeginRound( spell ):
	print "Resist Electricity OnBeginRound"

def OnEndSpellCast( spell ):
	print "Resist Electricity OnEndSpellCast"
	remove_from_persistent_list (RESIST_ELECTRICITY_KEY, spell.id)  # marc
		