from toee import *

def OnBeginSpellCast(spell):
    print "Sound Lance OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
    #game.particles("sp-evocation-conjure", spell.caster)

def OnSpellEffect(spell):
    print "Sound Lance OnSpellEffect"

def OnBeginRound(spell):
    print "Sound Lance OnBeginRound"

def OnBeginProjectile(spell, projectile, index_of_target):
    print "Sound Lance OnBeginProjectile"
    projectile.obj_set_int(obj_f_projectile_part_sys_id, game.particles('sp-Ray of Frost', projectile))

def OnEndProjectile( spell, projectile, index_of_target ):
    print "Sound Lance OnEndProjectile"

    spellDamageDice = dice_new('1d8')
    spellDamageDice.number = min(spell.caster_level, 10) #capped at CL 10

    spell.duration = 0

    game.particles_end(projectile.obj_get_int(obj_f_projectile_part_sys_id))
    spellTarget = spell.target_list[0]

    #Saving Throw for half damage; Sound Lance is not a ranged touch attack spell
    if spellTarget.obj.saving_throw_spell(spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id): #success
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
        spellTarget.obj.spell_damage_with_reduction(spell.caster, D20DT_SONIC, spellDamageDice, D20DAP_UNSPECIFIED, DAMAGE_REDUCTION_HALF, D20A_CAST_SPELL, spell.id)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
        spellTarget.obj.spell_damage(spell.caster, D20DT_SONIC, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnEndSpellCast(spell):
    print "Sound Lance OnEndSpellCast"