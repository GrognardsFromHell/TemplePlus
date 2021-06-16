from toee import *

def OnBeginSpellCast(spell):
    print "Slashing Darkness OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Slashing Darkness OnSpellEffect"

def OnBeginRound(spell):
    print "Slashing Darkness OnBeginRound"

def OnBeginProjectile(spell, projectile, index_of_target):
    print "Slashing Darkness OnBeginProjectile"
    projectile.obj_set_int(obj_f_projectile_part_sys_id, game.particles('sp-Ray of Enfeeblement', projectile))

def OnEndProjectile(spell, projectile, index_of_target):
    print "Slashing Darkness OnEndProjectile"

    spell.duration = 0
    spellTarget = spell.target_list[0]
    spellDamageDice = dice_new('1d8')
    spellDamageDice.number = min((spell.caster_level/2), 5) #capped at cl 10 (5d8)
    damageType = D20DT_NEGATIVE_ENERGY

    game.particles_end(projectile.obj_get_int(obj_f_projectile_part_sys_id))

    attackResult = spell.caster.perform_touch_attack(spellTarget.obj)

    if attackResult & D20CAF_HIT:
        game.particles('sp-Ray of Enfeeblement-END', spellTarget.obj)
        if attackResult & D20CAF_CRITICAL:
            spellDamageDice.number *= 2
        if spellTarget.obj.is_category_type(mc_type_undead):
            spellTarget.obj.spell_heal(spell.caster, spellDamageDice, D20A_CAST_SPELL, spell.id)
        else:
            spellTarget.obj.spell_damage(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30007)
        game.particles('Fizzle', spellTarget.obj)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnEndSpellCast(spell):
    print "Slashing Darkness OnEndSpellCast"

