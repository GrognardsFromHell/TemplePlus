from toee import *

import _include
from co8Util import size
from co8Util.PersistentData import *
from co8Util.ObjHandling import *

RM_KEY = "Sp404_RighteousMight_Activelist"

def OnBeginSpellCast( spell ):
        print "Righteous Might OnBeginSpellCast"
        print "spell.target_list=", spell.target_list
        print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
        game.particles( "sp-transmutation-conjure", spell.caster )


def OnSpellEffect( spell ):
        print "Righteous Might OnSpellEffect"

        spell.duration = 1 * spell.caster_level
        target_item = spell.target_list[0]

        target_item.obj.condition_add_with_args( 'sp-Righteous Might', spell.id, spell.duration, 0 )
        target_item.partsys_id = game.particles( 'sp-Righteous Might', target_item.obj )


def OnBeginRound( spell ):
        print "Righteous Might OnBeginRound"
        UndoCo8(spell)

def OnEndSpellCast( spell ):
        print "Righteous Might OnEndSpellCast"
        UndoCo8(spell)

def UndoCo8(spell):
        ##print "spell.target_list=", spell.target_list
        ##print "spell.id=", spell.id

        #size mod

        activeList = Co8PersistentData.getData(RM_KEY)
        if isNone(activeList):
                return

        for entry in activeList:
                spellID, target = entry
                targetObj = refHandle(target)
                #print "activeLIst Entry:" + str(spellID)
                if spellID == spell.id:
                        #print "Size:" + str(targetObj.obj_get_int(obj_f_size))
                        #print "Reach:" + str(targetObj.obj_get_int(obj_f_critter_reach))
                        size.resetSizeCategory(targetObj)
                        #print "resetting reach on", targetObj
                        #print "new Size:" + str(targetObj.obj_get_int(obj_f_size))
                        #print "new Reach:" + str(targetObj.obj_get_int(obj_f_critter_reach))
                        activeList.remove(entry)
                        #no more active spells
                        if len(activeList) == 0:
                                Co8PersistentData.removeData(RM_KEY)
                                break
    
                        Co8PersistentData.setData(RM_KEY, activeList)
                        break

        else: print "ERROR! Active RM spell without entry in activeList!"
