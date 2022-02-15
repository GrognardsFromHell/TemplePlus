from toee import *

def OnBeginSpellCast(spell):
    print "Ray of Dizziness OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
    game.particles("sp-evocation-conjure", spell.caster)

def OnSpellEffect(spell):
    print "Ray of Dizziness OnSpellEffect"

def OnBeginRound(spell):
    print "Ray of Dizziness OnBeginRound"

def OnBeginProjectile(spell, projectile, index_of_target):
    print "Ray of Dizziness OnBeginProjectile"
    projectile.obj_set_int(obj_f_projectile_part_sys_id, game.particles('sp-Ray of Frost', projectile))

def OnEndProjectile( spell, projectile, index_of_target ):
    print "Ray of Dizziness OnEndProjectile"

    spellTarget = spell.target_list[0]
    spell.duration = 1 * spell.caster_level # 1 round/casterlevel
    game.particles_end(projectile.obj_get_int(obj_f_projectile_part_sys_id))

    attackResult = spell.caster.perform_touch_attack(spellTarget.obj)

    if attackResult & D20CAF_HIT:
        spellTarget.partsys_id = game.particles('sp-Shout-Hit', spellTarget.obj)
        spellTarget.obj.condition_add_with_args('sp-Ray of Dizziness', spell.id, spell.duration)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30007)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)

    spell.spell_end(spell.id)

def OnEndSpellCast(spell):
    print "Ray of Dizziness OnEndSpellCast"