from toee import *

def OnBeginSpellCast(spell):
    print "Harmonic Chorus OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Harmonic Chorus OnSpellEffect"

    spell.duration = 1 * spell.caster_level # 1 round/cl
    spellTarget = spell.target_list[0]

    if spellTarget.obj.condition_add_with_args('sp-Harmonic Chorus', spell.id, spell.duration):
        spellTarget.partsys_id = game.particles('sp-Heroism', spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Harmonic Chorus OnBeginRound"

def OnEndSpellCast(spell):
    print "Harmonic Chorus OnEndSpellCast"