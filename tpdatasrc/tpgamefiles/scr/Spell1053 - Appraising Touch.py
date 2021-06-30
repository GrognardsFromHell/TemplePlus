from toee import *

def OnBeginSpellCast(spell):
    print "Appraising Touch OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Appraising Touch OnSpellEffect"

    spell.duration = 600 * spell.caster_level # 1 hour/cl
    spellTarget = spell.target_list[0]

    if spellTarget.obj.condition_add_with_args('sp-Appraising Touch', spell.id, spell.duration):
        spellTarget.partsys_id = game.particles('sp-Appraising Touch', spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Appraising Touch OnBeginRound"

def OnEndSpellCast(spell):
    print "Appraising Touch OnEndSpellCast"