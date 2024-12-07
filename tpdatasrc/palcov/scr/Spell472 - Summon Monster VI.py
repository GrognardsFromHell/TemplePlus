from toee import *
from utilities import *
from scripts import *

def OnBeginSpellCast( spell ):
	print "Summon Monster VI OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect ( spell ):
	print "Summon Monster VI OnSpellEffect"
	teststr = "; summon monster 6\n" #change this to the header line for the spell in spells_radial_menu_options.mes
	options = get_options_from_mes(teststr)
		
	spell.duration = 1 * spell.caster_level
	
	## Solves Radial menu problem for Wands/NPCs
	spell_arg = spell.spell_get_menu_arg( RADIAL_MENU_PARAM_MIN_SETTING )
	if spell_arg not in options:
		x = game.random_range(0,len(options)-1)
		spell_arg = options[x]

	# create monster, monster should be added to target_list
	spell.summon_monsters( 1, spell_arg)

	target_item = spell.target_list[0]
	game.particles('sp-Summon Monster V', target_item.obj)

	SummonMonster_Rectify_Initiative(spell, spell_arg) # Added by S.A. - sets iniative to caster's initiative -1, so that it gets to act in the same round
	
	spell.spell_end(spell.id)

def OnBeginRound( spell ):
	print "Summon Monster VI OnBeginRound"

def OnEndSpellCast( spell ):
	print "Summon Monster VI OnEndSpellCast"
	
