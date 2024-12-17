from toee import *
from utilities import *
from scripts import set_spell_flag, get_spell_flags, find_spell_obj_with_flag

## How this works:
##
## A variable is set in moduels\ToEE\delayed_blast_fireball.txt, that determines how many rounds the fireball is delayed.
## If it's 0, the spell works just like ordinary Fireball, except the damage is limited to 20d6 rather than 10d6.
## If it's greater than 0:
##	The spell's duration is set to the delay.
##	A "spell object" is created. (proto 6271; actually an armor object)
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

	if spell.caster.name in [14691]:        # Gelwea
		spell.dc = 17                       # 10 + 7 + 3
		spell.caster_level = 16

	game.particles( 'sp-Fireball-conjure', spell.caster )
	remove_list = []
	delay = 0
	dam = dice_new( '1d6' )
	dam.number = min( 20, spell.caster_level ) #edited by Allyx

#-	if game.global_vars[451] & 2**1 != 0:
#-		if game.leader.map == 5083:   ## Fire node - fire spells do x2 damage
#-			dam.number = dam.number * 2
#-		elif game.leader.map == 5084: ## Water node - fire spells do 1/2 damage
#-			dam.number = dam.number / 2

	print type(spell.target_loc)

	# delay for npc casters is in global_vars[66] set in the casters's script, marc
	if spell.caster.type != obj_t_pc and spell.caster.leader_get() == OBJ_HANDLE_NULL:
		delay = game.global_vars[66]
		game.global_vars[66] = 0
	else:
		ifile = file('delayed_blast_fireball.txt','r')
		str = ifile.readline()
		ifile.close()
		delay = int(str)

	print "delay = ", delay
	spell.duration = delay
	game.pfx_lightning_bolt( spell.caster, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y, spell.target_loc_off_z )

	if delay == 0: 

		xx,yy = location_to_axis(spell.target_loc)
		if game.leader.map == 5067 and ( xx >= 521 and xx <= 555 ) and ( yy >= 560 and yy <= 610):
		# Water Temple Pool Enchantment prevents fire spells from working inside the chamber, according to the module -SA
			tro = game.obj_create(14070, spell.target_loc)
			game.particles( 'swirled gas', spell.target_loc )
			tro.float_mesfile_line( 'mes\\skill_ui.mes', 2000 , 1 )
			tro.destroy()
			game.sound(7581,1)
			game.sound(7581,1)
			for target_item in spell.target_list:
				remove_list.append( target_item.obj )
			spell.target_list.remove_list( remove_list )
			spell.spell_end( spell.id )
			return

		game.particles( 'sp-Fireball-Hit', spell.target_loc )
		soundfizzle = 0


		for target_item in spell.target_list:
			xx,yy = location_to_axis(target_item.obj.location)
			if target_item.obj.map == 5067 and ( xx >= 521 and xx <= 555 ) and ( yy >= 560 and yy <= 610):
				# Water Temple Pool Enchantment prevents fire spells from working inside the chamber, according to the module -SA
				target_item.obj.float_mesfile_line( 'mes\\skill_ui.mes', 2000 , 1 )
				game.particles( 'swirled gas', target_item.obj )
				soundfizzle = 1
				remove_list.append( target_item.obj )
				continue

			if target_item.obj.reflex_save_and_damage( spell.caster, spell.dc, D20_Save_Reduction_Half, D20STD_F_NONE, dam, D20DT_FIRE, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id ):
				# saving throw successful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
			else:
				# saving throw unsuccessful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

			remove_list.append( target_item.obj )

		if soundfizzle == 1:
			game.sound(7581,1)
			game.sound(7581,1)

		spell.target_list.remove_list( remove_list )		
		spell.spell_end( spell.id )
		
	else:
		
		spell_obj = game.obj_create(6271, spell.target_loc)
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
			temp_obj = find_spell_obj_with_flag(c, 6271, 2147483647)
			if temp_obj != OBJ_HANDLE_NULL:
				if get_spell_flags(temp_obj) == spell.dc ^ spell.target_loc:
					spell_obj = game.obj_create(6271, c.location)
					temp_obj.destroy()
					break
				
	if spell_obj != OBJ_HANDLE_NULL:#if spell_obj is null here then it has gone beyond my reach (map change or too far away)
		targets = list(game.obj_list_cone( spell_obj, OLC_CRITTERS, 20, -180, 360 ))
		dam = dice_new('1d6')
		dam.number = min(20, spell.caster_level)
		if game.leader.map == 5083:   ## Fire node - fire spells do x2 damage
			dam.number = dam.number * 2
		elif game.leader.map == 5084: ## Water node - fire spells do 1/2 damage
			dam.number = dam.number / 2


		xx,yy = location_to_axis(spell_obj.location)
		if game.leader.map == 5067 and ( xx >= 521 and xx <= 555 ) and ( yy >= 560 and yy <= 610):
		# Water Temple Pool Enchantment prevents fire spells from working inside the chamber, according to the module -SA
			game.particles( 'swirled gas', spell_obj.location )
			spell_obj.float_mesfile_line( 'mes\\skill_ui.mes', 2000 , 1 )
			game.sound(7581,1)
			game.sound(7581,1)
			spell_obj.destroy()
			return

		game.particles( 'sp-Fireball-Hit', spell_obj.location )

		soundfizzle = 0
		for target_item in targets:
			xx,yy = location_to_axis(target_item.location)
			if target_item.map == 5067 and ( xx >= 521 and xx <= 555 ) and ( yy >= 560 and yy <= 610):
				# Water Temple Pool Enchantment prevents fire spells from working inside the chamber, according to the module -SA
				target_item.float_mesfile_line( 'mes\\skill_ui.mes', 2000 , 1 )
				game.particles( 'swirled gas', target_item )
				soundfizzle = 1
				continue
			if target_item.reflex_save_and_damage( spell.caster, spell.dc, D20_Save_Reduction_Half, D20STD_F_NONE, dam, D20DT_FIRE, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id ):
				# saving throw successful
				target_item.float_mesfile_line( 'mes\\spell.mes', 30001 )
			else:
				# saving throw unsuccessful
				target_item.float_mesfile_line( 'mes\\spell.mes', 30002 )

		if soundfizzle == 1:
			game.sound(7581,1)
			game.sound(7581,1)

		spell_obj.destroy()
	
	
