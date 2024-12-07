from toee import *

def OnBeginSpellCast( spell ):
	print "Fear OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	#game.particles( "sp-necromancy-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Harpy Song OnSpellEffect"

	#remove_list = []

	npc = spell.caster
	if npc.name == 14243:	## Harpy 	
		spell.dc = 16
		
	spell.duration = 5

	game.sound(4300, 1)

	for target_item in spell.target_list:
		if not target_item.obj in game.party[0].group_list():
			continue
		if not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):

			# saving throw unsuccessful
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

			#target_item.obj.condition_add_with_args( 'Fascinate', spell.id, spell.duration, 0 )
			target_item.obj.condition_add_with_args( 'Captivating Song', spell.id, spell.duration, 0,0,0,0,0,0 )
			# target_item.partsys_id = game.particles( 'sp-Fear-Hit', target_item.obj )

		else:

			# saving throw successful
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )


	#spell.target_list.remove_list( remove_list )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Song OnBeginRound"

def OnEndSpellCast( spell ):
	print "Song OnEndSpellCast"