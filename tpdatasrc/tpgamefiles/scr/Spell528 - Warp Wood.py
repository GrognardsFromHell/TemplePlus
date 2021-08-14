from toee import *
from utilities import *
from Co8 import *

def OnBeginSpellCast( spell ):
	print "Warp Wood OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-transmutation-conjure", spell.caster )

def OnSpellEffect ( spell ):
	print "Warp Wood OnSpellEffect"

	number_items = spell.caster_level
	remove_list = []
	itemref = 0
	print("Warp Wood: number_items org = " + str(number_items))
	for target_item in spell.target_list:
		
		if number_items > 0:

			# First, check creatures for held wooden items
			if (target_item.obj.type == obj_t_pc) or (target_item.obj.type == obj_t_npc):

				# Check for a wooden missile weapon and destroy that
				item_affected = target_item.obj.item_worn_at(3)
				print("Warp Wood: item_affected = " +str(item_affected) )
				print("  size = " + str(Warp_Size_by_Item_Size(item_affected)) )
				print("  number_items remaining = " + str(number_items) )

				if (item_affected.obj_get_int(obj_f_weapon_ammo_type) < 2) and (number_items >= Warp_Size_by_Item_Size(item_affected)):
					FLAGS = item_affected.item_flags_get()
					# Only works on nonmagic wood, and the item gets its owner's Will save
					if (item_affected.obj_get_int(obj_f_material) == mat_wood) and not(FLAGS & OIF_IS_MAGICAL) and not(target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id )):
						target_item.obj.float_mesfile_line( 'mes\\combat.mes', 22 )
						number_items = number_items - Warp_Size_by_Item_Size(item_affected)
						game.particles( 'Blight', target_item.obj )
						print("Destroying " + str(item_affected))
						item_affected.destroy()

				# Check for a wooden melee weapon and warp that (-4 to attack rolls)
				item_affected = target_item.obj.item_worn_at(3)

				if (item_affected != OBJ_HANDLE_NULL) and (number_items >= Warp_Size_by_Item_Size(item_affected)):
					FLAGS = item_affected.item_flags_get()
					# Only works on nonmagic wood, and the item gets its owner's Will save
					if (item_affected.obj_get_int(obj_f_material) == mat_wood) and not(FLAGS & OIF_IS_MAGICAL) and not(target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id )):
						if not(is_spell_flag_set(item_affected, OSF_IS_SUMMONED)):
							item_affected.item_condition_add_with_args('To Hit Bonus', -4)
							set_spell_flag(item_affected, OSF_IS_SUMMONED)
							game.particles( 'Blight', target_item.obj )
							target_item.obj.float_mesfile_line( 'mes\\combat.mes', 8 )
						number_items = number_items - Warp_Size_by_Item_Size(item_affected)
				
			# Done with creature checks

			# Check for unattended weapons targeted and warp, unwarp, or destroy them
			elif (target_item.obj.type == obj_t_weapon):
				item_affected = target_item.obj
				FLAGS = item_affected.item_flags_get()
				# Only works on nonmagical wooden items
				if (item_affected.obj_get_int(obj_f_material) == mat_wood) and not(FLAGS & OIF_IS_MAGICAL) and (number_items >= Warp_Size_by_Item_Size(item_affected)):
					# Missile weapons are destroyed
					if (item_affected.obj_get_int(obj_f_weapon_ammo_type) < 2):
						target_item.obj.float_mesfile_line( 'mes\\combat.mes', 22 )
						game.particles( 'Blight', target_item.obj )
						number_items = number_items - Warp_Size_by_Item_Size(item_affected)
						item_affected.destroy()
					# Melee weapons that are warped are replaced with an unwarped version
					elif (is_spell_flag_set(item_affected, OSF_IS_SUMMONED)):
						proto = item_affected.name
						holder = game.obj_create(proto, item_affected.location)
						number_items = number_items - Warp_Size_by_Item_Size(item_affected)
						item_affected.destroy()
					# Melee weapons that are unwarped are warped (-4 to attack rolls)
					else:
						item_affected.item_condition_add_with_args('To Hit Bonus', -4)
						set_spell_flag(item_affected, OSF_IS_SUMMONED)
						game.particles( 'Blight', target_item.obj )
						number_items = number_items - Warp_Size_by_Item_Size(item_affected)
						target_item.obj.float_mesfile_line( 'mes\\combat.mes', 8 )
				number_items = number_items - Warp_Size_by_Item_Size(item_affected)
					

			# Check for locked wooden chests, and unlock them
			elif (target_item.obj.type == obj_t_container):
				item_affected = target_item.obj
				if (target_item.obj.obj_get_int(obj_f_material) == mat_wood):
					if (target_item.obj.container_flags_get() & OCOF_LOCKED) and (number_items >= Warp_Size_by_Item_Size(item_affected)):
						target_item.obj.container_flag_unset( OCOF_LOCKED )
						target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30004 )
						number_items = number_items - Warp_Size_by_Item_Size(item_affected)
				else:
					game.particles( 'Fizzle', target_item.obj )
					target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30003 )

			# Check for locked wooden doors, and unlock and open them
			elif (target_item.obj.type == obj_t_portal):
				if (target_item.obj.obj_get_int(obj_f_material) == mat_wood):
					if (target_item.obj.portal_flags_get() & OPF_LOCKED) and (number_items >= Warp_Size_by_Item_Size(item_affected)):
						target_item.obj.portal_flag_unset( OPF_LOCKED )
						target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30004 )
						if not (target_item.obj.portal_flags_get() & OPF_OPEN):
							target_item.obj.portal_toggle_open()
							target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30013 )
						number_items = number_items - Warp_Size_by_Item_Size(item_affected)
				else:
					game.particles( 'Fizzle', target_item.obj )
					target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30003 )

			else:
				game.particles( 'Fizzle', target_item.obj )
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30003 )

		remove_list.append( target_item.obj )

	spell.target_list.remove_list( remove_list )		
	spell.spell_end(spell.id)

def OnBeginRound( spell ):
	print "Warp Wood OnBeginRound"

def OnEndSpellCast( spell ):
	print "Warp Wood OnEndSpellCast"

def Warp_Size_by_Item_Size( itemref ):
	# This function returns the equivalent number of warp-items used up for the spell
	# based on the passed object's size. A Small or smaller object counts as 1 item.
	# A Medium object counts as 2 items, a Large object as 4 items, and so on
	# up to 32 items for a Colossal object.
	size = itemref.obj_get_int(obj_f_size)
	if size == STAT_SIZE_MEDIUM:
		warp_size = 2
	elif size == STAT_SIZE_LARGE:
		warp_size = 4
	elif size == STAT_SIZE_HUGE:
		warp_size = 8
	elif size == STAT_SIZE_GARGANTUAN:
		warp_size = 16
	elif size >= STAT_SIZE_COLOSSAL:
		warp_size = 32
	else:
		warp_size = 1
	return warp_size