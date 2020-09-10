from toee import *
import _include
from co8Util import size
from co8Util.PersistentData import *
from co8Util.ObjHandling import *

GREASE_KEY = "Sp200_Grease_Activelist"


def OnBeginSpellCast( spell ):
	print "Grease OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Grease OnSpellEffect"

	spell.duration = 1 * spell.caster_level

	# spawn one spell_object object
	spell_obj = game.obj_create( OBJECT_SPELL_GENERIC, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y )

	# add to d20initiative
	caster_init_value = spell.caster.get_initiative()
	spell_obj.d20_status_init()
	spell_obj.set_initiative( caster_init_value )

	# put sp-Grease condition on obj
	spell_obj_partsys_id = game.particles( 'sp-Small-Grease', spell_obj )
	spell_obj.condition_add_with_args( 'sp-Grease', spell.id, spell.duration, 0, spell_obj_partsys_id )
	#spell_obj.condition_add_arg_x( 3, spell_obj_partsys_id )
	#objectevent_id = spell_obj.condition_get_arg_x( 2 )

	
	#########################
	# Added by Sitra Achara	#
	#########################

	spell_obj.obj_set_int( obj_f_secretdoor_dc, 200 + (1<<15)   ) 
	# Mark it as a "grease" object. 
	# 1<<15 - marks it as "active"
	# bits 16 and onward - random ID number
	
	activeList = Co8PersistentData.getData(GREASE_KEY)
	if isNone(activeList): activeList = []
	activeList.append([spell.id, derefHandle(spell_obj)])
	Co8PersistentData.setData(GREASE_KEY, activeList)


	#########################
	# End of Section		#
	#########################	
	
def OnBeginRound( spell ):
	print "Grease OnBeginRound"

def OnEndSpellCast( spell ):
	print "Grease OnEndSpellCast"

	activeList = Co8PersistentData.getData(GREASE_KEY)
	if isNone(activeList):
		print "ERROR! Active Grease spell without activeList!"
		return

	for entry in activeList:
		spellID, target = entry
		targetObj = refHandle(target)
		if spellID == spell.id:
			aaa = targetObj.obj_get_int( obj_f_secretdoor_dc )
			aaa &= ~(1<<15)
			targetObj.obj_set_int( obj_f_secretdoor_dc, aaa )
			activeList.remove(entry)
			#no more active spells
			if len(activeList) == 0:
				Co8PersistentData.removeData(GREASE_KEY)
				break
    
                	Co8PersistentData.setData(GREASE_KEY, activeList)
			break
def OnAreaOfEffectHit( spell ):
	print "Grease OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Grease OnSpellStruck"