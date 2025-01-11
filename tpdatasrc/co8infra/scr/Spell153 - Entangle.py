from toee import *
import _include
from co8Util import size
from co8Util.PersistentData import *
from co8Util.ObjHandling import *
import tpdp

ENTANGLE_KEY = "Sp153_Entangle_Activelist"

def OnBeginSpellCast( spell ):
	print "Entangle OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-transmutation-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Entangle OnSpellEffect"

	spell.duration = 10 * spell.caster_level
	outdoor_map_list = [5001, 5002, 5009, 5042, 5043, 5051, 5062, 5068, 5069, 5070, 5071, 5072, 5073, 5074, 5075, 5076, 5077, 5078, 5091, 5093, 5094, 5095, 5096, 5097, 5099, 5100, 5108, 5110, 5111, 5112, 5113, 5119, 5120, 5121, 5132, 5142, 5189]

	if (tpdp.config_get_bool("StricterRulesEnforcement") and tpdp.is_temple_module()) or (game.leader.map in outdoor_map_list):

		# spawn one Entangle scenery object
		entangle_obj = game.obj_create( OBJECT_SPELL_GENERIC, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y )

		# add to d20initiative
		caster_init_value = spell.caster.get_initiative()
		entangle_obj.d20_status_init()
		entangle_obj.set_initiative( caster_init_value )

		# put sp-Entangle condition on obj
		entangle_partsys_id = game.particles( 'sp-Entangle-Area', entangle_obj )
		entangle_obj.condition_add_with_args( 'sp-Entangle', spell.id, spell.duration, 0, entangle_partsys_id )
		#entangle_obj.condition_add_arg_x( 3, entangle_partsys_id )
		#objectevent_id = entangle_obj.condition_get_arg_x( 2 )

		#########################
		# Added by Sitra Achara	#
		#########################

		entangle_obj.obj_set_int( obj_f_secretdoor_dc, 153 + (1<<15) ) 
		# Mark it as an "obscuring mist" object. 
		# 1<<15 - marks it as "active"
		# bits 16 and onward - random ID number

		activeList = Co8PersistentData.getData(ENTANGLE_KEY)
		if isNone(activeList): activeList = []
		activeList.append([spell.id, derefHandle(entangle_obj)])
		Co8PersistentData.setData(ENTANGLE_KEY, activeList)

		#########################
		# End of Section	#
		#########################

	else:
		# No plants to entangle with
		game.particles( 'Fizzle', spell.caster )
		spell.caster.float_mesfile_line( 'mes\\spell.mes', 30000 )
		spell.caster.float_mesfile_line( 'mes\\spell.mes', 16014 )

def OnBeginRound( spell ):
	print "Entangle OnBeginRound"

def OnEndSpellCast( spell ):
	print "Entangle OnEndSpellCast"

	activeList = Co8PersistentData.getData(ENTANGLE_KEY)
	if isNone(activeList):
		print "ERROR! Active Entangle spell without activeList!"
		return

	for entry in activeList:
		spellID, target = entry
		targetObj = refHandle(target)
		if spellID == spell.id:
			aaa = targetObj.obj_get_int( obj_f_secretdoor_dc )
			aaa &= ~(1<<15)
			targetObj.obj_set_int( obj_f_secretdoor_dc, aaa )
			activeList.remove(entry)
			# no more active spells
			if len(activeList) == 0:
				Co8PersistentData.removeData(ENTANGLE_KEY)
				break
			Co8PersistentData.setData(ENTANGLE_KEY, activeList)
			break

def OnAreaOfEffectHit( spell ):
	print "Entangle OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Entangle OnSpellStruck"