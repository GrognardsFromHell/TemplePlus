from toee import *

def OnBeginSpellCast(spell):
    print "Sticky Fingers OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Sticky Fingers OnSpellEffect"

    spell.duration = 1 #1 round, I am unsure if this easy to handle in the game!!
    spellTarget = spell.target_list[0]

    if spellTarget.obj.condition_add_with_args('sp-Sticky Fingers', spell.id, spell.duration):
        spellTarget.partsys_id = game.particles('sp-Sticky Fingers', spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Sticky Fingers OnBeginRound"

def OnEndSpellCast(spell):
    print "Sticky Fingers OnEndSpellCast"