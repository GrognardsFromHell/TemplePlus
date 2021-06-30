from toee import *

def OnBeginSpellCast(spell):
    print "Frost Breath OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Frost Breath OnSpellEffect"

    targetsToRemove = []
    spell.duration = 1 #spell deals instant damage, but can add a 1 round daze effect on a failed save
    saveType = D20_Save_Reflex
    saveDescriptor = D20STD_F_NONE
    spellDamageDice = dice_new('1d4')
    spellDamageDice.number = min((spell.caster_level/2), 5) #capped at CL 10
    damageType = D20DT_COLD

    game.particles('sp-Frost Breath', spell.caster)
    for spellTarget in spell.target_list:
        #Saving throw for half damage and avoid being dazed
        if spellTarget.obj.saving_throw_spell(spell.dc, saveType, saveDescriptor, spell.caster, spell.id): #success
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
            spellTarget.obj.spell_damage_with_reduction(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, DAMAGE_REDUCTION_HALF, D20A_CAST_SPELL, spell.id)
            targetsToRemove.append(spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
            spellTarget.obj.spell_damage(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id)
            spellTarget.obj.condition_add_with_args('sp-Daze', spell.id, spell.duration)

    if targetsToRemove:
        spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Frost Breath OnBeginRound"

def OnEndSpellCast(spell):
    print "Frost Breath OnEndSpellCast"

