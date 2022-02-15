from toee import *

def OnBeginSpellCast(spell):
    print "Loves Lament OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Loves Lament OnSpellEffect"
    
    targetsToRemove = []
    spell.duration = 0

    wisdomDamageDice = dice_new('1d6')
    nauseatedDurationDice = dice_new('1d4')

    game.particles('sp-Sound Burst', spell.caster)

    for spellTarget in spell.target_list:
        #Save to negate spell
        if spellTarget.obj.saving_throw_spell(spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id): #success
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
            game.particles('Fizzle', spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
            wisdomDamage = wisdomDamageDice.roll()
            spellTarget.obj.condition_add_with_args("Temp_Ability_Loss", stat_wisdom, wisdomDamage)
            nauseatedDuration = nauseatedDurationDice.roll()
            persistentFlag = 0
            spellTarget.obj.condition_add_with_args("Nauseated", nauseatedDuration, persistentFlag, 0)
        targetsToRemove.append(spellTarget.obj)

    spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

    
def OnBeginRound(spell):
    print "Loves Lament OnBeginRound"

def OnEndSpellCast(spell):
    print "Loves Lament OnEndSpellCast"