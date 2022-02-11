from toee import *

def OnBeginSpellCast(spell):
    print "Orb of Sound OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Orb of Sound OnSpellEffect"

def OnBeginRound(spell):
    print "Orb of Sound OnBeginRound"

def OnBeginProjectile(spell, projectile, index_of_target):
    print "Orb of Sound OnBeginProjectile"
    projectile.obj_set_int(obj_f_projectile_part_sys_id, game.particles("sp-Orb of Cold-proj", projectile))

def OnEndProjectile(spell, projectile, index_of_target):
    print "Orb of Sound OnEndProjectile"

    spell.duration = 0
    spellTarget = spell.target_list[0]
    spellDamageDice = dice_new('1d6')
    spellDamageDice.number = min(spell.caster_level, 15) #capped at cl 15 (15d6)
    damageType = D20DT_SONIC
    spellDamageReduction = 100 #100 indicates full damage

    game.particles_end(projectile.obj_get_int(obj_f_projectile_part_sys_id))

    attackResult = spell.caster.perform_touch_attack(spellTarget.obj)

    if attackResult & D20CAF_HIT:
        game.particles("sp-Lesser Orb of Cold", spellTarget.obj)
        spellTarget.obj.spell_damage_weaponlike(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, spellDamageReduction, D20A_CAST_SPELL, spell.id, attackResult, index_of_target)
        #Save for secondary effect
        if spellTarget.obj.saving_throw_spell(spell.dc, D20_Save_Fortitude, D20STD_F_SPELL_DESCRIPTOR_COLD, spell.caster, spell.id):
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
            game.particles('Fizzle', spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
            duration = 1
            #sp-Deafness is permanent currently, I'll add a helper condition to remove it after it should expire
            #as there is no condition_remove in python
            if spellTarget.obj.condition_add_with_args("sp-Deafness", spell.id, spell.duration, 0):
                spellTarget.partsys_id = game.particles("sp-Blindness-Deafness", spellTarget.obj)
                spellTarget.obj.condition_add_with_args("Remove Deafness Helper Condition", duration, 0)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30007)
        game.particles('Fizzle', spellTarget.obj)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnEndSpellCast(spell):
    print "Orb of Sound OnEndSpellCast"

