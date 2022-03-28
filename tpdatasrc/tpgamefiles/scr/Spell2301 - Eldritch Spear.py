from toee import *

def OnBeginSpellCast(spell):
    print "Eldritch Spear OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
    print "spell.spell_level=", spell.spell_level
    print "spell.dc=", spell.dc

def OnSpellEffect(spell):
    print "Eldritch Spear OnSpellEffect"

def OnBeginRound(spell):
    print "Eldritch Spear OnBeginRound"

def OnBeginProjectile(spell, projectile, index_of_target):
    print "Eldritch Spear OnBeginProjectile"
    projectile.obj_set_int(obj_f_projectile_part_sys_id, game.particles("sp-Eldritch Spear-proj", projectile))

def OnEndProjectile(spell, projectile, index_of_target):
    print "Eldritch Spear OnEndProjectile"

    spell.duration = 0
    spellTarget = spell.target_list[index_of_target]
    spellDamageDice = dice_new("1d6")
    spellDamageDice.number = min(int(min((spell.caster_level + 1) / 2, 6)) + int(max((spell.caster_level - 11) / 3, 0)), 9) #capped at 9d6 at cl 20
    damageType = spell.caster.d20_query("PQ_Eldritch_Blast_Damage_Type")
    if not damageType:
        damageType = D20DT_MAGIC
    spellDamageReduction = 100 #100 indicates full damage

    game.particles_end(projectile.obj_get_int(obj_f_projectile_part_sys_id))

    attackResult = spell.caster.perform_touch_attack(spellTarget.obj)
    
    if attackResult & D20CAF_HIT:
        game.particles("sp-Eldritch Blast-hit", spellTarget.obj)
        spellTarget.obj.spell_damage_weaponlike(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, spellDamageReduction, D20A_CAST_SPELL, spell.id, attackResult, index_of_target)
        if not spellTarget.obj.d20_query("PQ_Eldritch_Blast_Has_Secondary_Effect"):
            spell.target_list.remove_target(spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30007)
        game.particles("Fizzle", spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)


def OnEndSpellCast(spell):
    print "Eldritch Spear OnEndSpellCast"

