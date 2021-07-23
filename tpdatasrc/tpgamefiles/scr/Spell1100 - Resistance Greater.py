from toee import *

def OnBeginSpellCast(spell):
    print "Resistance Greater OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Resistance Greater OnSpellEffect"

    spell.duration = 14400 # 24h
    spellTarget = spell.target_list[0]

    if spellTarget.obj.condition_add_with_args('sp-Resistance Greater', spell.id, spell.duration):
        spellTarget.partsys_id = game.particles('sp-Resistance', spell.caster)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Resistance Greater OnBeginRound"

def OnEndSpellCast(spell):
    print "Resistance Greater OnEndSpellCast"