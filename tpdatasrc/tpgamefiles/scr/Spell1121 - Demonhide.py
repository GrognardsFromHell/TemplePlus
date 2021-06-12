from toee import *

def OnBeginSpellCast(spell):
    print "Demonhide OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Demonhide OnSpellEffect"

    spell.duration = 1 * spell.caster_level # 1 round/level
    spellTarget = spell.target_list[0]
    spellTargetAlignment = spellTarget.obj.stat_level_get(stat_alignment)

    if spellTargetAlignment & ALIGNMENT_EVIL: #Demonhide works only on evil targets
        if spellTarget.obj.condition_add_with_args('sp-Demonhide', spell.id, spell.duration):
            spellTarget.partsys_id = game.particles('sp-Demonhide', spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
            spell.target_list.remove_target(spellTarget.obj)
            spell.spell_end(spell.id)
    else:
        spellTarget.obj.float_text_line("Not evil", tf_red)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Demonhide OnBeginRound"

def OnEndSpellCast(spell):
    print "Demonhide OnEndSpellCast"