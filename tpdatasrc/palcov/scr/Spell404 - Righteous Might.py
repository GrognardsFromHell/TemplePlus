from toee import *

import _include
from co8Util import size
from co8Util.PersistentData import *
from co8Util.ObjHandling import *
from co8Util.spells import *

#RM_KEY = "Sp404_RighteousMight_Activelist"

def OnBeginSpellCast( spell ):
	print "Righteous Might OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-transmutation-conjure", spell.caster )


def OnSpellEffect( spell ):
	print "Righteous Might OnSpellEffect"

	spell.duration = 1 * spell.caster_level
	target_item = spell.target_list[0]

	if has_spell_condition (target_item.obj, RIGHTEOUS_MIGHT_KEY):  # to prevent spell stacking, marc
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 16007 )
		game.particles( 'Fizzle', target_item.obj )
	else:

		#size mod
	##	print "Size:" + str(target_item.obj.obj_get_int(obj_f_size))
	##	print "Reach:" + str(target_item.obj.obj_get_int(obj_f_critter_reach))
		size.incSizeCategory(target_item.obj)

		#save target_list
		activeList = Co8PersistentData.getData(RIGHTEOUS_MIGHT_KEY)
		if isNone(activeList): activeList = []
		activeList.append([spell.id, derefHandle(target_item.obj)])
		Co8PersistentData.setData(RIGHTEOUS_MIGHT_KEY, activeList)
	
	##	print "new Size:" + str(target_item.obj.obj_get_int(obj_f_size))
	##	print "new Reach:" + str(target_item.obj.obj_get_int(obj_f_critter_reach))

		target_item.obj.condition_add_with_args( 'sp-Righteous Might', spell.id, spell.duration, 0 )
		target_item.partsys_id = game.particles( 'sp-Righteous Might', target_item.obj )

def OnBeginRound( spell ):
	print "Righteous Might OnBeginRound"

def OnEndSpellCast( spell ):
	print "Righteous Might OnEndSpellCast"
##	print "spell.target_list=", spell.target_list
##	print "spell.id=", spell.id

	#size mod

	activeList = Co8PersistentData.getData(RIGHTEOUS_MIGHT_KEY)
	if isNone(activeList):
		print "ERROR! Active RM spell without activeList!"
		return

	for entry in activeList:
		spellID, target = entry
		targetObj = refHandle(target)
		if spellID == spell.id:
##			print "Size:" + str(targetObj.obj_get_int(obj_f_size))
##			print "Reach:" + str(targetObj.obj_get_int(obj_f_critter_reach))
			size.resetSizeCategory(targetObj)
##			print "resetting reach on", targetObj
##			print "new Size:" + str(targetObj.obj_get_int(obj_f_size))
##			print "new Reach:" + str(targetObj.obj_get_int(obj_f_critter_reach))
			activeList.remove(entry)
			#no more active spells
			if len(activeList) == 0:
				Co8PersistentData.removeData(RIGHTEOUS_MIGHT_KEY)
				break
			#save new activeList
		Co8PersistentData.setData(RIGHTEOUS_MIGHT_KEY, activeList)
		break

	else: print "ERROR! Active RM spell without entry in activeList!"
