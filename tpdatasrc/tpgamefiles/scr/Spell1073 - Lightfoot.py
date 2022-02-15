from toee import *

def OnBeginSpellCast(spell):
    print "Lightfoot OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Lightfoot OnSpellEffect"
    
    spell.duration = 0 #Lightfoot is a Swift Action, works in the current round
    spellTarget = spell.target_list[0]

    if spellTarget.obj.condition_add_with_args('sp-Lightfoot', spell.id, spell.duration, 0):
        spellTarget.partsys_id = game.particles('sp-Expeditious Retreat', spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Lightfoot OnBeginRound"

def OnEndSpellCast(spell):
    print "Lightfoot OnEndSpellCast"