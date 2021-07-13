from toee import *
from Co8 import *

import _include
from co8Util import size
from co8Util.PersistentData import *
from co8Util.ObjHandling import *

ENLARGE_KEY = "Sp152_Enlarge_Activelist"

def OnBeginSpellCast( spell ):
	print "Enlarge OnBeginSpellCast\n"
	#print "\nspell.target_list=" , str(spell.target_list) , "\n"
	#print "\nspell.caster=" + str( spell.caster) + " caster.level= ", spell.caster_level , "\n"
	#print "\nspell.id=", spell.id , "\n"
	game.particles( "sp-transmutation-conjure", spell.caster )


def OnSpellEffect( spell ):
	print "Enlarge OnSpellEffect\n"
##	print "spell.id=", spell.id
##	print "spell.target_list=", spell.target_list
	spell.duration = 10 * spell.caster_level
	target_item = spell.target_list[0]

	# HTN - 3.5, enlarge PERSON only
	if ( target_item.obj.is_category_type( mc_type_humanoid ) == 1 ):
		if target_item.obj.is_friendly( spell.caster ):
			return_val = target_item.obj.condition_add_with_args( 'sp-Enlarge', spell.id, spell.duration, 0 )
#			print "condition_add_with_args return_val: " + str(return_val) + "\n"
			if return_val == 1:
				#size mod
#				print "Size:" + str(target_item.obj.obj_get_int(obj_f_size))
#				print "Reach:" + str(target_item.obj.obj_get_int(obj_f_critter_reach))
				size.incSizeCategory(target_item.obj)
#				print "performed size Increase\n"
				#save target_list
				activeList = Co8PersistentData.getData(ENLARGE_KEY)
				if isNone(activeList): activeList = []
				activeList.append([spell.id, derefHandle(target_item.obj)])
				Co8PersistentData.setData(ENLARGE_KEY, activeList)
				
#				print "new Size:" + str(target_item.obj.obj_get_int(obj_f_size))
#				print "new Reach:" + str(target_item.obj.obj_get_int(obj_f_critter_reach))


				target_item.partsys_id = game.particles( 'sp-Enlarge', target_item.obj )
			else:
				# sp-Enlarge not applied, probably dispelled sp-Reduce
				spell.target_list.remove_target(target_item.obj)
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
					# sp-Enlarge not applied, probably dispelled sp-Reduce
					spell.target_list.remove_target(target_item.obj)
			else:
				# saving throw successful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

				game.particles( 'Fizzle', target_item.obj )
				spell.target_list.remove_target( target_item.obj )
	else: # not a humanoid
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30000 )
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31004 )

		game.particles( 'Fizzle', target_item.obj )
		spell.target_list.remove_target( target_item.obj )

	print "spell.target_list=", spell.target_list
	spell.spell_end( spell.id )


def OnBeginRound( spell ):
	print "Enlarge OnBeginRound"
	return


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

