from toee import *

def OnBeginSpellCast(spell):
    print "Allegro OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Allegro OnSpellEffect"
    
    targetsToRemove = []
    spell.duration = 10 * spell.caster_level # 1 min/cl

    for spellTarget in spell.target_list:
        if spellTarget.obj.condition_add_with_args('sp-Allegro', spell.id, spell.duration):
            spellTarget.partsys_id = game.particles('sp-Expeditious Retreat', spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
            targetsToRemove.append(spellTarget.obj)

    if targetsToRemove:
        spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Allegro OnBeginRound"

def OnEndSpellCast(spell):
    print "Allegro OnEndSpellCast"