from toee import *

def OnBeginSpellCast( spell ):
	print "Resist Elements OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Resist Elements OnSpellEffect"

	spell_arg = spell.spell_get_menu_arg( RADIAL_MENU_PARAM_MIN_SETTING )

	## Solves Radial menu problem for Wands/NPCs
	spell_arg = spell.spell_get_menu_arg( RADIAL_MENU_PARAM_MIN_SETTING )
	if spell_arg != 1 and spell_arg != 2 and spell_arg != 3 and spell_arg != 4 and spell_arg != 5:
		spell_arg = game.random_range(1,5)

	npc = spell.caster
	if npc.name == 8047:			##  Alrrem
		spell_arg = 4
	if npc.name == 8035:			##  added so NPC's can cast spell
		spell_arg = 3
	if npc.name == 14336:			##  added so NPC's can cast spell
		spell_arg = 4

	if spell.target_list[0].obj.name == 14195 or spell.target_list[0].obj.name == 14224: # Oohlgrist or Aern; they already has a magic ring against fire
		spell_arg = 2

	if spell_arg == 1:
		element_type = ACID
		partsys_type = 'sp-Resist Elements-acid'
	elif spell_arg == 2:
		element_type = COLD
		partsys_type = 'sp-Resist Elements-cold'
	elif spell_arg == 3:
		element_type = ELECTRICITY
		partsys_type = 'sp-Resist Elements-water'
	elif spell_arg == 4:
		element_type = FIRE
		partsys_type = 'sp-Resist Elements-fire'
	elif spell_arg == 5:
		element_type = SONIC
		partsys_type = 'sp-Resist Elements-sonic'

	spell.duration = 100 * spell.caster_level

	npc = spell.caster			##  added so NPC's can pre-buff
	if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active():
		spell.duration = 2000 * spell.caster_level

	target_item = spell.target_list[0]

	if target_item.obj.condition_add_with_args( 'sp-Resist Elements', spell.id, element_type , spell.duration):
		target_item.partsys_id = game.particles( partsys_type, target_item.obj )
	#target_item.obj.condition_add_with_args( 'sp-Resist Elements', spell.id, element_type, spell.duration )
	#target_item.partsys_id = game.particles( partsys_type, target_item.obj )

	#spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Resist Elements OnBeginRound"

def OnEndSpellCast( spell ):
	print "Resist Elements OnEndSpellCast"