from toee import *
import _include
from co8Util import size
from co8Util.PersistentData import *
from co8Util.ObjHandling import *

WEB_KEY = "Sp531_Web_Activelist"

def OnBeginSpellCast( spell ):
	print "Web OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Web OnSpellEffect"

	spell.duration = 100 * spell.caster_level

	# spawn one Web scenery object
	web_obj = game.obj_create( OBJECT_SPELL_GENERIC, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y )

	# add to d20initiative
	caster_init_value = spell.caster.get_initiative()
	web_obj.d20_status_init()
	web_obj.set_initiative( caster_init_value )

	# put sp-Web condition on obj
	Web_partsys_id = game.particles( 'sp-Web', web_obj )
	web_obj.condition_add_with_args( 'sp-Web', spell.id, spell.duration, 0, Web_partsys_id )
	#web_obj.condition_add_arg_x( 3, Web_partsys_id )
	#objectevent_id = web_obj.condition_get_arg_x( 2 )
	
	#########################
	# Added by Sitra Achara	#
	#########################
	
	web_obj.obj_set_int( obj_f_secretdoor_dc, 531 + (1<<15)   ) 
	# Mark it as an "obscuring mist" object. 
	# 1<<15 - marks it as "active"
	# bits 16 and onward - random ID number

	activeList = Co8PersistentData.getData(WEB_KEY)
	if isNone(activeList): activeList = []
	activeList.append([spell.id, derefHandle(web_obj)])
	Co8PersistentData.setData(WEB_KEY, activeList)

	#########################
	# End of Section		#
	#########################
	

def OnBeginRound( spell ):
	print "Web OnBeginRound"

def OnEndSpellCast( spell ):
	print "Web OnEndSpellCast"
	
	
	activeList = Co8PersistentData.getData(WEB_KEY)
	if isNone(activeList):
		print "ERROR! Active Web spell without activeList!"
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
				Co8PersistentData.removeData(WEB_KEY)
				break

				Co8PersistentData.setData(WEB_KEY, activeList)
			break

def OnAreaOfEffectHit( spell ):
	print "Web OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Web OnSpellStruck"