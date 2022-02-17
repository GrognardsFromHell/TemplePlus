from toee import *

def OnBeginSpellCast(spell):
    print "Aid Mass OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Aid Mass OnSpellEffect"

    targetsToRemove = []
    spell.duration = 10 * spell.caster_level

    tempHpDice = dice_new('1d8')
    tempHpDice.bonus = min(spell.caster_level, 15)

    for spellTarget in spell.target_list:
        tempHpAmount = tempHpDice.roll()
        if spellTarget.obj.condition_add_with_args('sp-Aid Mass', spell.id, spell.duration, tempHpAmount, 0):
            spellTarget.partsys_id = game.particles('sp-Aid', spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
            targetsToRemove.append(spellTarget.obj)

    if targetsToRemove:
        spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Aid Mass OnBeginRound"

def OnEndSpellCast(spell):
    print "Aid Mass OnEndSpellCast"

