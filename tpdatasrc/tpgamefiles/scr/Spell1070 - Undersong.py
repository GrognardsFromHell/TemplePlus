from toee import *

def OnBeginSpellCast(spell):
    print "Undersong OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Undersong OnSpellEffect"

    spell.duration = 100 * spell.caster_level # 10 min/cl
    spellTarget = spell.target_list[0]

    if spellTarget.obj.condition_add_with_args('sp-Undersong', spell.id, spell.duration):
        spellTarget.partsys_id = game.particles('sp-Undersong', spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Undersong OnBeginRound"

def OnEndSpellCast(spell):
    print "Undersong OnEndSpellCast"