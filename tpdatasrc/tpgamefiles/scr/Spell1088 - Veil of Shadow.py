from toee import *

def OnBeginSpellCast(spell):
    print "Veil of Shadow OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Veil of Shadow OnSpellEffect"

    spell.duration = 10 * spell.caster_level # 1 min/cl
    spellTarget = spell.target_list[0]

    #Veil of Shadow is dispelled by daylight
    if game.is_outdoor() and game.is_daytime():
        spellTarget.obj.float_text_line("Dispelled by daylight")
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
    else:
        if spellTarget.obj.condition_add_with_args('sp-Veil of Shadow', spell.id, spell.duration):
            spellTarget.partsys_id = game.particles('sp-Veil of Shadow', spell.caster)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
            spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Veil of Shadow OnBeginRound"

def OnEndSpellCast(spell):
    print "Veil of Shadow OnEndSpellCast"