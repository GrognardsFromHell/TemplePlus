from toee import *

def OnBeginSpellCast(spell):
    print "Rhinos Rush OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Rhinos Rush OnSpellEffect"
    
    spell.duration = 0 #current round
    spellTarget = spell.target_list[0]

    if spellTarget.obj.condition_add_with_args('sp-Rhinos Rush', spell.id, spell.duration, 0):
        spellTarget.partsys_id = game.particles('sp-Bullstrength', spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)


def OnBeginRound(spell):
    print "Rhinos Rush OnBeginRound"

def OnEndSpellCast(spell):
    print "Rhinos Rush OnEndSpellCast"