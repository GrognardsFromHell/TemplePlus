from toee import *

def OnBeginSpellCast(spell):
    print "Hunter's Eye OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Hunter's Eye OnSpellEffect"
    
    spell.duration = 0 #Hunter's Eye is a Swift Action, works in the current round
    spellTarget = spell.target_list[0]
    bonusDice = spell.caster_level/3

    if spellTarget.obj.condition_add_with_args("sp-Hunter's Eye", spell.id, spell.duration, bonusDice, 0):
        spellTarget.partsys_id = game.particles("sp-Hunter's Eye", spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Hunter's Eye OnBeginRound"

def OnEndSpellCast(spell):
    print "Hunter's Eye OnEndSpellCast"