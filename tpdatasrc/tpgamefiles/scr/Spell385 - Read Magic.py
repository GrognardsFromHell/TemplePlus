from toee import *

def OnBeginSpellCast( spell ):
	print "Read Magic OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	#game.particles( "sp-divination-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Read Magic OnSpellEffect"

	spell.duration = 0

	target_item = spell.target_list[0]

	game.particles( 'sp-Read Magic', spell.caster )
	target_item.obj.item_flag_set( OIF_IDENTIFIED )

	spell.target_list.remove_target( target_item.obj )
	spell.spell_end( spell.id )
	
	# NEW! Identifies all scrolls and potions when cast in safe area
	if (spell.caster in game.party):	
		import random_encounter
		sleepStatus = random_encounter.can_sleep()
		if (sleepStatus == SLEEP_SAFE or sleepStatus == SLEEP_PASS_TIME_ONLY):
			for dude in game.party:
				for q in range(0, 24):
					item = dude.inventory_item(q)
					if (item != OBJ_HANDLE_NULL):
						if (item.type == obj_t_food or item.type == obj_t_scroll):
							item.item_flag_set(OIF_IDENTIFIED)
	
	
def OnBeginRound( spell ):
	print "Read Magic OnBeginRound"

def OnEndSpellCast( spell ):
	print "Read Magic OnEndSpellCast"