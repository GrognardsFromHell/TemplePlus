from toee import *

def OnBeginSpellCast(spell):
    print "Accuracy OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Accuracy OnSpellEffect"

    spell.duration = 100 * spell.caster_level # 10 mins/level
    spellTarget = spell.target_list[0]

    if spellTarget.obj.condition_add_with_args('sp-Accuracy', spell.id, spell.duration):
        spellTarget.partsys_id = game.particles('sp-Sure Strike', spell.caster)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Accuracy OnBeginRound"

def OnEndSpellCast(spell):
    print "Accuracy OnEndSpellCast"