from toee import *

def OnBeginSpellCast(spell):
    print "Sirines Grace OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Sirines Grace OnSpellEffect"

    spell.duration = 1 * spell.caster_level # 1 round/level
    spellTarget = spell.target_list[0]
    bonusValue = max(0, int((spell.caster.stat_level_get(stat_charisma) - 10 ) / 2))

    if spellTarget.obj.condition_add_with_args('sp-Sirines Grace', spell.id, spell.duration, bonusValue, 0):
        spellTarget.partsys_id = game.particles('sp-Heroism', spell.caster)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Sirines Grace OnBeginRound"

def OnEndSpellCast(spell):
    print "Sirines Grace OnEndSpellCast"