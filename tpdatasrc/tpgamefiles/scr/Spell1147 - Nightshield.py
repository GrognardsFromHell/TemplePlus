from toee import *

def OnBeginSpellCast(spell):
    print "Nightshield OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Nightshield OnSpellEffect"

    spell.duration = 10 * spell.caster_level
    spellTarget = spell.target_list[0]
    if spell.caster_level < 6:
        spellBonus = 1
    elif spell.caster_level < 9:
        spellBonus = 2
    else:
        spellBonus = 3

    if spellTarget.obj.condition_add_with_args('sp-Nightshield', spell.id, spell.duration, spellBonus):
        spellTarget.partsys_id = game.particles('sp-Shield', spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Nightshield OnBeginRound"

def OnEndSpellCast(spell):
    print "Nightshield OnEndSpellCast"

