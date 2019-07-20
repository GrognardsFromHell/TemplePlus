from toee import *
from utilities import *
from Co8 import *

## How this works:
##
## A variable is set in moduels\ToEE\delayed_blast_fireball.txt, that determines how many rounds the fireball is delayed.
## If it's 0, the spell works just like ordinary Fireball, except the damage is limited to 20d6 rather than 10d6.
## If it's greater than 0:
##	The spell's duration is set to the delay.
##	A "spell object" is created. (proto 6400; actually an armor object)
##	This spell object is assigned a randomized ID, and borks the dc ???????
##	When the number of rounds equal to the delay passes, the spell "ends".
##	This triggers the OnEndSpellCast() script.
##	The script looks for the spell object in the spell caster's vicinity.
##		NB: This means that changing maps or walking far away will bork the spell.	
##	If the spell object is found:
##		Target everything in its vicinity using a 360 degree targeting cone, with a reach of 20.
##		Asplode!
##	Possibly enlarge spell, maximize spell etc won't work?
##	Now given the Water Temple pool treatment

def OnBeginSpellCast( spell ):
	print "Delayed Blast Fireball OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Delayed Blast Fireball OnSpellEffect"
	game.particles( 'sp-Fireball-conjure', spell.caster )
	remove_list = []
	delay = 0
	dam = dice_new( '1d6' )
	dam.number = min( 20, spell.caster_level ) #edited by Allyx
	print type(spell.target_loc)
	delay = 0 # removed delay scripting because it used open(). Who uses that anyway? TODO - make this a radial menu selection (talk about low priority...)
	print "delay = ", delay
	spell.duration = delay
	game.pfx_lightning_bolt( spell.caster, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y, spell.target_loc_off_z )
	
	if delay == 0:
		game.particles( 'sp-Fireball-Hit', spell.target_loc )
		for target_item in spell.target_list:
			if target_item.obj.reflex_save_and_damage( spell.caster, spell.dc, D20_Save_Reduction_Half, D20STD_F_NONE, dam, D20DT_FIRE, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id ):
				# saving throw successful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
			else:
				# saving throw unsuccessful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

			remove_list.append( target_item.obj )
		spell.target_list.remove_list( remove_list )		
		spell.spell_end( spell.id )
		
	else:
		
		spell_obj = game.obj_create(6400, spell.target_loc)
		spell_obj.item_flag_unset(OIF_NO_DROP)		
		for target_item in spell.target_list:
			remove_list.append( target_item.obj)
		spell.target_list.remove_list( remove_list )
		spell.num_of_targets = 1
		spell.target_list[0].obj = spell.caster			
		spell.target_list[0].obj.condition_add_with_args('sp-Mage Armor', spell.id, spell.duration, 0)
		# spell.dc = game.random_range(0, 2147483647)
		spell.target_loc = game.random_range(0,2147483647)
		set_spell_flag(spell_obj, spell.dc ^ spell.target_loc)
		
		
		
def OnBeginRound( spell ):
	print "Delayed Blast Fireball OnBeginRound"

def OnEndSpellCast( spell ):
	print "Delayed Blast Fireball OnEndSpellCast"
	spell_obj = OBJ_HANDLE_NULL
	object_list =  list(game.obj_list_cone( spell.caster, OLC_ARMOR, 200, -180, 360 ))
	for t in object_list: #find the blast object in the area...
		if get_spell_flags(t) == spell.dc ^ spell.target_loc:
			spell_obj = t
	if spell_obj == OBJ_HANDLE_NULL:#okay not there, maybe someone picked it up.... 
		pot_carriers = list(game.obj_list_cone( spell.caster, OLC_CRITTERS, 200, -180, 360 ))
		for c in pot_carriers:
			temp_obj = find_spell_obj_with_flag(c, 6400, 2147483647)
			if temp_obj != OBJ_HANDLE_NULL:
				if get_spell_flags(temp_obj) == spell.dc ^ spell.target_loc:
					spell_obj = game.obj_create(6400, c.location)
					temp_obj.destroy()
					break
				
	if spell_obj != OBJ_HANDLE_NULL:#if spell_obj is null here then it has gone beyond my reach (map change or too far away)
		targets = list(game.obj_list_cone( spell_obj, OLC_CRITTERS, 20, -180, 360 ))
		game.particles( 'sp-Fireball-Hit', spell_obj.location )
		dam = dice_new('1d6')
		dam.number = min(20, spell.caster_level)
		for target_item in targets:
			if target_item.reflex_save_and_damage( spell.caster, spell.dc, D20_Save_Reduction_Half, D20STD_F_NONE, dam, D20DT_FIRE, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id ):
				# saving throw successful
				target_item.float_mesfile_line( 'mes\\spell.mes', 30001 )
			else:
				# saving throw unsuccessful
				target_item.float_mesfile_line( 'mes\\spell.mes', 30002 )
		spell_obj.destroy()
	
	
