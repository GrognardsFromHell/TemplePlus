from toee import *

def OnBeginSpellCast(spell):
    print "Conviction Mass OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Conviction Mass OnSpellEffect"

    targetsToRemove = []
    spell.duration = 100 * spell.caster_level
    spellBonus = min((2 + (spell.caster_level/6)), 5) #Capped at CL 18 for a total bonus of +5

    for spellTarget in spell.target_list:
        if spellTarget.obj.is_friendly(spell.caster):
            if spellTarget.obj.condition_add_with_args('sp-Conviction', spell.id, spell.duration, spellBonus):
                spellTarget.partsys_id = game.particles('sp-Heroism', spellTarget.obj)
            else:
                spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
                game.particles('Fizzle', spellTarget.obj)
                targetsToRemove.append(spellTarget.obj)
        else:
            targetsToRemove.append(spellTarget.obj)

    if targetsToRemove:
        spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Conviction Mass OnBeginRound"

def OnEndSpellCast(spell):
    print "Conviction Mass OnEndSpellCast"

