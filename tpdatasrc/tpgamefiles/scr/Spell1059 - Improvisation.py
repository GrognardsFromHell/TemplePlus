from toee import *

def OnBeginSpellCast(spell):
    print "Improvisation OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Improvisation OnSpellEffect"

    spell.duration = spell.caster_level #1 round/cl
    spellTarget = spell.target_list[0]
    bonusPool = spell.caster_level * 2 #Luck Pool is twice casterlevel
    bonusToAdd = spell.caster_level/2 #single bonus cannot exeed half casterlevel

    spellTarget.obj.condition_add_with_args('sp-Improvisation', spell.id, spell.duration, bonusToAdd, bonusPool, 0, 0, 0)
    spellTarget.partsys_id = game.particles('sp-Heroism', spellTarget.obj)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Improvisation OnBeginRound"

def OnEndSpellCast(spell):
    print "Improvisation OnEndSpellCast"