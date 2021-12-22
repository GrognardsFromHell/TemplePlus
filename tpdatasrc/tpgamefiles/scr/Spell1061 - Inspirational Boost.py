from toee import *

def OnBeginSpellCast(spell):
    print "Inspirational Boost OnBeginSpellCast"
    print "spell.spellTarget_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Inspirational Boost OnSpellEffect"

    spell.duration = 1 # Inspire Courage must be used in the same round
    spellTarget = spell.target_list[0]

    spellTarget.obj.condition_add_with_args('sp-Inspirational Boost', spell.id, spell.duration, 0, 0, 0)
    spellTarget.partsys_id = game.particles('Bardic-Inspire Greatness', spellTarget.obj )

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Inspirational Boost OnBeginRound"

def OnEndSpellCast(spell):
    print "Inspirational Boost OnEndSpellCast"