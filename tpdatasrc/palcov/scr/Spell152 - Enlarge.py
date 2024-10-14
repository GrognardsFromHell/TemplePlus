from toee import *

import _include
from co8Util import size
from co8Util.PersistentData import *
from co8Util.ObjHandling import *
from co8Util.spells import *

#ENLARGE_KEY = "Sp152_Enlarge_Activelist"

def OnBeginSpellCast( spell ):
	print "Enlarge OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	print "spell.id=", spell.id
	game.particles( "sp-transmutation-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Enlarge OnSpellEffect"
##	print "spell.id=", spell.id
##	print "spell.target_list=", spell.target_list
	spell.duration = 10 * spell.caster_level
	target_item = spell.target_list[0]

	# HTN - 3.5, enlarge PERSON only
	if ( target_item.obj.is_category_type( mc_type_humanoid ) == 1 ):
		if target_item.obj.is_friendly( spell.caster ):
			return_val = target_item.obj.condition_add_with_args( 'sp-Enlarge', spell.id, spell.duration, 0 )
			if return_val == 1:
				#size mod
##				print "Size:" + str(target_item.obj.obj_get_int(obj_f_size))
##				print "Reach:" + str(target_item.obj.obj_get_int(obj_f_critter_reach))
				size.incSizeCategory(target_item.obj)
				
				#save target_list
				activeList = Co8PersistentData.getData(ENLARGE_KEY)
				if isNone(activeList): activeList = []
				activeList.append([spell.id, derefHandle(target_item.obj)])
				Co8PersistentData.setData(ENLARGE_KEY, activeList)
				
##				print "new Size:" + str(target_item.obj.obj_get_int(obj_f_size))
##				print "new Reach:" + str(target_item.obj.obj_get_int(obj_f_critter_reach))


				target_item.partsys_id = game.particles( 'sp-Enlarge', target_item.obj )
		else:
			if not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id ):

				# saving throw unsuccesful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

				return_val = target_item.obj.condition_add_with_args( 'sp-Enlarge', spell.id, spell.duration, 0 )
				#enemies seem to work fine?
	##								print "Size:" + str(target_item.obj.obj_get_int(obj_f_size))
	##								print "Reach:" + str(target_item.obj.obj_get_int(obj_f_critter_reach))
	##								size.incSizeCategory(target_item.obj)
	##								print "new Size:" + str(target_item.obj.obj_get_int(obj_f_size))
	##								print "new Reach:" + str(target_item.obj.obj_get_int(obj_f_critter_reach))
				if return_val == 1:
					target_item.partsys_id = game.particles( 'sp-Enlarge', target_item.obj )
				else:
					# saving throw successful
					target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

					game.particles( 'Fizzle', target_item.obj )
	#								spell.target_list.remove_target( target_item.obj )
	print "spell.target_list=", spell.target_list
#	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Enlarge OnBeginRound"

def OnEndSpellCast( spell ):
	#strangely enough the target_list gets zeroed before entering this, not after
	print "Enlarge OnEndSpellCast"
##	print "spell.target_list=", spell.target_list
##	print "spell.id=", spell.id

	#size mod

	activeList = Co8PersistentData.getData(ENLARGE_KEY)
	if isNone(activeList):
		print "ERROR! Active Enlarge spell without activeList!"
		return

	for entry in activeList:
		spellID, target = entry
		targetObj = refHandle(target)
		if spellID == spell.id:
##			print "Size:" + str(targetObj.obj_get_int(obj_f_size))
##			print "Reach:" + str(targetObj.obj_get_int(obj_f_critter_reach))
			weap_too_big(targetObj)
			size.resetSizeCategory(targetObj)
##			print "resetting reach on", targetObj
##			print "new Size:" + str(targetObj.obj_get_int(obj_f_size))
##			print "new Reach:" + str(targetObj.obj_get_int(obj_f_critter_reach))
			activeList.remove(entry)
			#no more active spells
			if len(activeList) == 0:
				Co8PersistentData.removeData(ENLARGE_KEY)
				break
			#save new activeList
			Co8PersistentData.setData(ENLARGE_KEY, activeList)
			break

	else: print "ERROR! Active Enlarge spell without entry in activeList!"


#------------------------------------------------------------------------------
# Originally in Co8.py
#------------------------------------------------------------------------------

#################################################################
#Updated by Shiningted 19/9/9                                   #
#################################################################
def weap_too_big(weap_user):
	if weap_user.is_category_type( mc_type_giant ):
		return
	weap_1 = weap_user.item_worn_at(3)
	weap_2 = weap_user.item_worn_at(4)
	# added these 4 lines to check for shield too, marc
	# buckler is fine
	shield_too_big = 0
	shield = weap_user.item_worn_at(11)
	if shield.obj_get_int(obj_f_size) > STAT_SIZE_SMALL:
		shield_too_big = 1
	size_1 = weap_1.obj_get_int(obj_f_size)
	if size_1 > STAT_SIZE_MEDIUM and (weap_2 != OBJ_HANDLE_NULL or shield_too_big):
		unequip( 3, weap_user)
	size_2 = weap_2.obj_get_int(obj_f_size)
	if size_2 > STAT_SIZE_MEDIUM:
		unequip( 4, weap_user)
	return
#################################################################
#End added by Shiningted                                        #
#################################################################

###################################################################
#Added by Darmagon                                                #
###################################################################
def unequip( slot, npc, whole_party = 0):
	unequip_set = []
	if whole_party:
		unequip_set = game.party
	else:
		unequip_set = [npc]
	for npc2 in unequip_set:
		i = 0
		j = 0
		item = npc2.item_worn_at(slot)
		if item != OBJ_HANDLE_NULL:
			if item.item_flags_get() & OIF_NO_DROP:
				item.item_flag_unset(OIF_NO_DROP)
				i = 1
			if item.item_flags_get() & OIF_NO_TRANSFER:
				item.item_flag_unset(OIF_NO_TRANSFER)
				j = 1
			holder = game.obj_create(1004, npc2.location) 
			holder.item_get(item)
			tempp = npc2.item_get(item)
			pc_index = 0
			# this part is insurance against filled up inventory for any PCs
			while tempp == 0 and pc_index < len(game.party):
				if game.party[pc_index].type == obj_t_pc:
					tempp = game.party[pc_index].item_get(item)
				pc_index += 1
			if i:
				item.item_flag_set(OIF_NO_DROP)
			if j:
				item.item_flag_set(OIF_NO_TRANSFER)
			# marc, in case every party members inventory slot is full
			if tempp != 0:
				holder.destroy()
#################################################################
#End added by Darmagon                                          #
#################################################################
