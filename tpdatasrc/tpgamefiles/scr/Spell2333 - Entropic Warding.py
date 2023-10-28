from toee import *

def OnBeginSpellCast(spell):
    print "Entropic Warding OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Entropic Warding OnSpellEffect"

    spell.duration = 10 * spell.caster_level #1 min/cl
    spellTarget = spell.target_list[0]

    if spellTarget.obj.condition_add_with_args("sp-Entropic Shield", spell.id, spell.duration, 0):
        spellTarget.partsys_id = game.particles("sp-Entropic Shield", spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30000)
        game.particles("Fizzle", spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Entropic Warding OnBeginRound"

def OnEndSpellCast(spell):
    print "Entropic Warding OnEndSpellCast"

