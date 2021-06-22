from toee import *

def OnBeginSpellCast(spell):
    print "Serene Visage OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Serene Visage OnSpellEffect"

    spell.duration = 10 * spell.caster_level # 1 Minute/cl
    spellTarget = spell.target_list[0]

    bonusValue = min((spell.caster_level/2), 10) #Bonus is capped at 10
    bonusValue = max(1, bonusValue) #Minimum 1

    if spellTarget.obj.condition_add_with_args('sp-Serene Visage', spell.id, spell.duration, bonusValue):
        spellTarget.partsys_id = game.particles('sp-Serene Visage', spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Serene Visage OnBeginRound"

def OnEndSpellCast(spell):
    print "Serene Visage OnEndSpellCast"