from toee import *

def OnBeginSpellCast(spell):
    print "Awaken Sin OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Awaken Sin OnSpellEffect"

    spellTarget = spell.target_list[0]
    spell.duration = 0
    saveType = D20_Save_Will
    saveDescriptor = D20STD_F_NONE
    spellDamageDice = dice_new('1d6')
    spellDamageDice.number = min(spell.caster_level, 10) #capped at CL 10
    damageType = D20DT_SUBDUAL
    #Target needs an intelligence score of 3+ and needs to be evil
    intelligentEnough = True if spellTarget.obj.stat_level_get(stat_intelligence) >= 3 else False
    isEvil = True if spellTarget.obj.stat_level_get(stat_alignment) & ALIGNMENT_EVIL else False

    if intelligentEnough and isEvil:
        game.particles('sp-Enervation-hit', spellTarget.obj)
        #Saving Throw to negate; Awaken Sin is not a touch attack spell
        if spellTarget.obj.saving_throw_spell(spell.dc, saveType, saveDescriptor, spell.caster, spell.id): #success
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
            game.particles('Fizzle', spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
            spellTarget.obj.spell_damage(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id)
            spellTarget.obj.float_text_line("Stunned", tf_red)
            stunDuration = 1
            spellTarget.obj.condition_add_with_args('Stunned', stunDuration, 0)
            #if unconscious after damage from awaken sin, additional wisdom damage
            if spellTarget.obj.is_unconscious():
                wisdomDamageDice = dice_new('1d6')
                wisdomDamageDice.number = 1
                wisdomDamage = wisdomDamageDice.roll()
                spellTarget.obj.condition_add_with_args('Temp_Ability_Loss', stat_wisdom, wisdomDamage)
    else:
        spellTarget.obj.float_text_line("Immune", tf_red)
        game.particles('Fizzle', spellTarget.obj)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Awaken Sin OnBeginRound"

def OnEndSpellCast(spell):
    print "Awaken Sin OnEndSpellCast"