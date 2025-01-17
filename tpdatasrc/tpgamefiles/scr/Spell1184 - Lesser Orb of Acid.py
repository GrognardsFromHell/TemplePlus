from toee import *

def OnBeginSpellCast(spell):
    print "Lesser Orb of Acid OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Lesser Orb of Acid OnSpellEffect"

def OnBeginRound(spell):
    print "Lesser Orb of Acid OnBeginRound"

def OnBeginProjectile(spell, projectile, index_of_target):
    print "Lesser Orb of Acid OnBeginProjectile"
    projectile.obj_set_int(obj_f_projectile_part_sys_id, game.particles("sp-Lesser Orb of Acid-proj", projectile))

def OnEndProjectile(spell, projectile, index_of_target):
    print "Lesser Orb of Acid OnEndProjectile"

    spell.duration = 0
    spellTarget = spell.target_list[0]
    spellDamageDice = dice_new('1d8')
    spellDamageDice.number = min((spell.caster_level + 1) / 2, 5) #capped at cl 9 (5d8)
    damageType = D20DT_ACID
    spellDamageReduction = 100 #100 indicates full damage

    game.particles_end(projectile.obj_get_int(obj_f_projectile_part_sys_id))

    attackResult = spell.caster.perform_touch_attack(spellTarget.obj)

    if attackResult & D20CAF_HIT:
        game.particles("sp-Lesser Orb of Acid", spellTarget.obj)
        spellTarget.obj.spell_damage_weaponlike(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, spellDamageReduction, D20A_CAST_SPELL, spell.id, attackResult, index_of_target)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30007)
        game.particles('Fizzle', spellTarget.obj)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnEndSpellCast(spell):
    print "Lesser Orb of Acid OnEndSpellCast"

