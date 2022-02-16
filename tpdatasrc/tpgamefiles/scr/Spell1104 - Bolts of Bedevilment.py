from toee import *

def OnBeginSpellCast(spell):
    print "Bolts of Bedevilment OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Bolts of Bedevilment OnSpellEffect"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

    spell.duration = 1 * spell.caster_level # 1 round/cl
    spellTarget = spell.target_list[0]
    numberOfCharges = -1

    spellTarget.obj.condition_add_with_args('sp-Bolts of Bedevilment', spell.id, spell.duration, numberOfCharges, spell.dc, 0)
    #spellTarget.partsys_id = game.particles('sp-Bolts of Bedevilment', spellTarget.obj)

def OnBeginRound(spell):
    print "Bolts of Bedevilment OnBeginRound"

def OnEndSpellCast(spell):
    print "Bolts of Bedevilment OnEndSpellCast"