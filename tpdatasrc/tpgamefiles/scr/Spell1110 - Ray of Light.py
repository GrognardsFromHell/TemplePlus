from toee import *

def OnBeginSpellCast(spell):
    print "Ray of Light OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Ray of Light OnSpellEffect"

def OnBeginRound(spell):
    print "Ray of Light OnBeginRound"

def OnBeginProjectile(spell, projectile, index_of_target):
    print "Ray of Light OnBeginProjectile"
    projectile.obj_set_int(obj_f_projectile_part_sys_id, game.particles('sp-Ray of Frost', projectile))

def OnEndProjectile( spell, projectile, index_of_target ):
    print "Ray of Light OnEndProjectile"

    spellTarget = spell.target_list[0]
    spell.duration = 0
    game.particles_end(projectile.obj_get_int(obj_f_projectile_part_sys_id))

############   Weapon Focus Ray Fix   ############
    spell.caster.condition_add('Wf Ray Fix', 0)
############ Weapon Focus Ray Fix End ############

    if spell.caster.perform_touch_attack(spellTarget.obj) & D20CAF_HIT:
        spellTarget.partsys_id = game.particles('sp-Arcane Eye-END', spellTarget.obj)
        blindnessDurationDice = dice_new('1d4')
        blindnessDuration = blindnessDurationDice.roll()
        spellTarget.obj.condition_add('Blindness', blindnessDuration)
        spellTarget.obj.float_text_line("Blinded", tf_red)
        game.create_history_freeform("{} is ~blinded~[TAG_BLINDED]\n\n".format(spellTarget.obj.description))
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30007)
        game.particles('Fizzle', spellTarget.obj)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnEndSpellCast(spell):
    print "Ray of Light OnEndSpellCast"