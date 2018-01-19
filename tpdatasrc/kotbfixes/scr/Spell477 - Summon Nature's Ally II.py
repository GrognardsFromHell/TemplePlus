from toee import *
from utilities import *


def OnBeginSpellCast( spell ):
	print "Summon Nature's II OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect ( spell ):
	print "Summon Nature's II OnSpellEffect"
	teststr = "; summon nature's ally 2\n" #change this to the header line for the spell in spells_radial_menu_options.mes
	options = get_options_from_mes(teststr)
		
	spell.duration = 1 * spell.caster_level
	
	## Solves Radial menu problem for Wands/NPCs
	spell_arg = spell.spell_get_menu_arg( RADIAL_MENU_PARAM_MIN_SETTING )
	if spell_arg not in options:
		x = game.random_range(0,len(options)-1)
		spell_arg = options[x]

	# create monster, monster should be added to target_list
	spell.summon_monsters( 1, spell_arg)

	spell.spell_end(spell.id)

def OnBeginRound( spell ):
	print "Summon Nature's II OnBeginRound"

def OnEndSpellCast( spell ):
	print "Summon Nature's II OnEndSpellCast"
	
def get_options_from_mes(teststr):
	print "get_options_from_mes"
	options = []
	i_file = open('data\\mes\\spells_radial_menu_options.mes','r')
	s = i_file.readline()
	while s != teststr and s !='':
		s = i_file.readline()
	s = i_file.readline()
	str_list = s.split()
	num_str = str_list[1]
	num_str = num_str.strip()
	num_str = num_str.replace("{","")
	num_str = num_str.replace("}","")
	num_options = int(num_str)
	i = 0
	while i < num_options:
		s = i_file.readline()
		str_list = s.split()
		num_str = str_list[1]
		num_str = num_str.strip()
		num_str = num_str.replace("{","")
		num_str = num_str.replace("}","")
		options.append(int(num_str))
		i = i + 1
	i_file.close()
	return options