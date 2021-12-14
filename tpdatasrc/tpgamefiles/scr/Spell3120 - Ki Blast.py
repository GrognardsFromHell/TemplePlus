from toee import *

def OnBeginSpellCast(spell):
    print "Ki Blast OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Ki Blast OnSpellEffect"

def OnBeginRound(spell):
    print "Ki Blast OnBeginRound"

def OnBeginProjectile(spell, projectile, index_of_target):
    print "Ki Blast OnBeginProjectile"

def OnEndProjectile(spell, projectile, index_of_target):
    print "Ki Blast OnEndProjectile"
    print "spell.target_list=", spell.target_list
    print "spell.id=", spell.id
    game.particles_end(projectile.obj_get_int(obj_f_projectile_part_sys_id))

    spell.duration = 0
    spellTarget = spell.target_list[index_of_target]
    damageType = D20DT_FORCE
    spellDamageDice = dice_new('1d6')
    spellDamageDice.number = 3
    spellDamageDice.bonus = (spell.caster.stat_level_get(stat_wisdom) - 10) / 2

    attackResult = spell.caster.perform_touch_attack(spellTarget.obj)
    if attackResult & D20CAF_HIT:
        game.particles("ft-Ki Blast-Hit", spellTarget.obj)
        spellTarget.obj.spell_damage_weaponlike(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, 100, D20A_CAST_SPELL, spell.id, attackResult, index_of_target)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30007)
        game.particles('Fizzle', spellTarget.obj)

    spell.target_list.remove_target(spellTarget.obj)
    spell.num_of_projectiles -= 1
    if spell.num_of_projectiles == 0:
        spell.spell_end(spell.id, 1)

def OnEndSpellCast(spell):
    print "Ki Blast OnEndSpellCast"