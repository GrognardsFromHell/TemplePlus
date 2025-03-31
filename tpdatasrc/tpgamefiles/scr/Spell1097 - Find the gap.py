from toee import *

def OnBeginSpellCast(spell):
    print "Find the Gap OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Find the Gap OnSpellEffect"

    spell.duration = 1 * spell.caster_level # 1 round/cl
    spellTarget = spell.target_list[0]

    if spellTarget.obj.condition_add_with_args('sp-Find the Gap', spell.id, spell.duration, 0, 0):
        spellTarget.partsys_id = game.particles('sp-True Strike', spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Find the Gap OnBeginRound"

def OnEndSpellCast(spell):
    print "Find the Gap OnEndSpellCast"