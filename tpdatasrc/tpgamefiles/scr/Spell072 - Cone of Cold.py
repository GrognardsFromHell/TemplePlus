from toee import *

def OnBeginSpellCast( spell ):
    print "Cone of Cold OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
    game.particles( "sp-evocation-conjure", spell.caster )

def OnSpellEffect( spell ):
    print "Cone of Cold OnSpellEffect"

    targetsToRemove = []
    spell.duration = 0
    saveType = D20_Save_Reflex
    saveDescriptor = D20STD_F_NONE
    spellDamageDice = dice_new('1d6')
    spellDamageDice.number = min(spell.caster_level, 15)
    damageType = D20DT_COLD

    game.particles( 'sp-Cone of Cold', spell.caster )
    for spellTarget in spell.target_list:
        #Saving throw for half damage
        if spellTarget.obj.saving_throw_spell(spell.dc, saveType, saveDescriptor, spell.caster, spell.id): #success
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
            spellTarget.obj.spell_damage_with_reduction(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, DAMAGE_REDUCTION_HALF, D20A_CAST_SPELL, spell.id)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
            spellTarget.obj.spell_damage(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id)
        targetsToRemove.append(spellTarget.obj)

    if targetsToRemove:
        spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound( spell ):
    print "Cone of Cold OnBeginRound"

def OnEndSpellCast( spell ):
    print "Cone of Cold OnEndSpellCast"