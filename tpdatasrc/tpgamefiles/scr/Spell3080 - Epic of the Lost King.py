from toee import *

def OnBeginSpellCast(spell):
    print "Epic of the Lost King Breath OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
    game.particles("sp-abjuration-conjure", spell.caster) #placeholder

def OnSpellEffect(spell):
    print "Epic of the Lost King Breath OnSpellEffect"

    targetsToRemove = []
    spell.duration = 0

    #game.particles(particleEffect, spell.caster)

    for spellTarget in spell.target_list:
        if spell.spell_level == 1:
            spellTarget.obj.condition_add_with_args("sp-Remove Fatigue", spell.id, spell.duration, 0)
        elif spell.spell_level == 3:
            spellTarget.obj.condition_add_with_args("sp-Remove Exhaustion", spell.id, spell.duration, 0)
        #spellTarget.partsys_id = game.particles("sp-Rage-END", spellTarget.obj) #placeholder
        targetsToRemove.append(spellTarget.obj)

    spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Epic of the Lost King Breath OnBeginRound"

def OnEndSpellCast(spell):
    print "Epic of the Lost King Breath OnEndSpellCast"