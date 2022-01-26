from toee import *

def OnBeginSpellCast(spell):
    print "Sure Strike OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Sure Strike OnSpellEffect"
    
    spell.duration = 0 #Sure Strike is a Swift Action, works in the current round
    spellTarget = spell.target_list[0]
    bonusValue = spell.caster_level/3

    if spellTarget.obj.condition_add_with_args('sp-Sure Strike', spell.id, spell.duration, bonusValue, 0):
        spellTarget.partsys_id = game.particles('sp-Sure Strike', spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Sure Strike OnBeginRound"

def OnEndSpellCast(spell):
    print "Sure Strike OnEndSpellCast"