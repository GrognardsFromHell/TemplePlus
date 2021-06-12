from toee import *

def OnBeginSpellCast(spell):
    print "Focusing Chant OnBeginSpellCast"
    print "spell.spellTarget_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Focusing Chant OnSpellEffect"

    spell.duration = 10 # 1 Minute
    spellTarget = spell.target_list[0]

    spellTarget.obj.condition_add_with_args('sp-Focusing Chant', spell.id, spell.duration)
    spellTarget.partsys_id = game.particles('sp-Focusing Chant', spellTarget.obj)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Focusing Chant OnBeginRound"

def OnEndSpellCast(spell):
    print "Focusing Chant OnEndSpellCast"