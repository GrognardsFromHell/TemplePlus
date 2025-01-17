from toee import *

def OnBeginSpellCast( spell ):
    print "Prismatic Ray OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
    game.particles( "sp-evocation-conjure", spell.caster )

def OnSpellEffect( spell ):
    print "Prismatic Ray OnSpellEffect"

def OnBeginRound( spell ):
    print "Prismatic Ray OnBeginRound"

def OnBeginProjectile(spell, projectile, index_of_target):
    print "Prismatic Ray OnBeginProjectile"
    projectile.obj_set_int(obj_f_projectile_part_sys_id, game.particles('sp-Prismatic Ray', projectile))

def OnEndProjectile( spell, projectile, index_of_target ):
    print "Prismatic Ray OnEndProjectile"

    spellTarget = spell.target_list[0]
    spell.duration = 0
    game.particles_end(projectile.obj_get_int(obj_f_projectile_part_sys_id))

    if spell.caster.perform_touch_attack(spellTarget.obj) & D20CAF_HIT:
        spellTarget.partsys_id = game.particles('sp-Prismatic Ray-Hit', spellTarget.obj)
        
        #Blindness effect for creatures less then 6 hit dice
        if spellTarget.obj.hit_dice_num < 6:
            blindnessDurationDice = dice_new('2d4')
            blindnessDuration = blindnessDurationDice.roll()
            if spellTarget.obj.condition_add('Blindness', blindnessDuration):
                spellTarget.obj.float_text_line("Blinded", tf_red)
                game.create_history_freeform("{} is ~blinded~[TAG_BLINDED]\n\n".format(spellTarget.obj.description))
                
        spell.duration = 1000
        add_prismatic_effect(spell, spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30007)
        game.particles('Fizzle', spellTarget.obj)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnEndSpellCast( spell ):
    print "Prismatic Ray OnEndSpellCast"
    
#Select a random effect similar to prismatic spray
def add_prismatic_effect(spell, t):
    effect = game.random_range(1,6)
    
    if effect == 1:
        print "Prismatic Spray Effect - Fire Damage"
    
        #20 fire damage
        dam = dice_new('1d1')
        dam.number = 20
        xx,yy = location_to_axis(t.location)
        if t.map == 5067 and ( xx >= 521 and xx <= 555 ) and ( yy >= 560 and yy <= 610):
            # Water Temple Pool Enchantment prevents fire spells from working inside the chamber, according to the module -SA
            t.float_mesfile_line( 'mes\\skill_ui.mes', 2000 , 1 )

            game.particles( 'swirled gas', t )
            game.sound(7581,1)
            game.sound(7581,1)
        else:
            if t.reflex_save_and_damage( spell.caster, spell.dc, D20_Save_Reduction_Half, D20STD_F_NONE, dam, D20DT_FIRE, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id ) > 0:
                # saving throw successful
                t.float_mesfile_line( 'mes\\spell.mes', 30001 )
            else:
                # saving throw unsuccessful
                t.float_mesfile_line( 'mes\\spell.mes', 30002 )
        
    elif effect == 2:
        print "Prismatic Spray Effect - Acid Damage"
    
        #40 acid damage
        dam = dice_new('1d1')
        dam.number = 40
        if t.reflex_save_and_damage( spell.caster, spell.dc, D20_Save_Reduction_Half, D20STD_F_NONE, dam, D20DT_ACID, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id ) > 0:
            # saving throw successful
            t.float_mesfile_line( 'mes\\spell.mes', 30001 )
        else:
            # saving throw unsuccessful
            t.float_mesfile_line( 'mes\\spell.mes', 30002 )
    elif effect == 3:
        print "Prismatic Spray Effect - Electricity Damage"
    
        #80 Electricity damage
        dam = dice_new('1d1')
        dam.number = 80
        if t.reflex_save_and_damage( spell.caster, spell.dc, D20_Save_Reduction_Half, D20STD_F_NONE, dam, D20DT_ELECTRICITY, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id ) > 0:
            # saving throw successful
            t.float_mesfile_line( 'mes\\spell.mes', 30001 )
        else:
            # saving throw unsuccessful
            t.float_mesfile_line( 'mes\\spell.mes', 30002 )
    elif effect == 4:
        print "Prismatic Spray Effect - Poisioned"
    
        #poisoned no save = die save = 1-10 con damage
        if t.is_category_type(mc_type_ooze) == 0 and t.is_category_type(mc_type_plant) == 0 and t.is_category_type(mc_type_undead) == 0 and t.is_category_type(mc_type_construct) == 0:

            if not t.saving_throw_spell( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id ):
                # saving throw unsuccessful
                t.float_mesfile_line( 'mes\\spell.mes', 30002 )
                # So you'll get awarded XP for the kill
                if not t in game.leader.group_list():
                    t.damage( game.leader , D20DT_UNSPECIFIED, dice_new( "1d1" ) )
                t.critter_kill_by_effect()
            else:
                # saving throw successful
                poison_index = 23
                time_to_secondary = 10
                poison_dc = 200
                t.float_mesfile_line( 'mes\\spell.mes', 30001 )
                t.condition_add_with_args( 'Poisoned', poison_index, time_to_secondary, poison_dc )
                game.timevent_add(end_poison,(spell, t), 6000)#don't want secondary damage here
                
        
    elif effect == 5:
        print "Prismatic Spray Effect - Turned to Stone"
        
		#turned to stone (effect is a long lasting hold like prismatic spray)
        if t.saving_throw( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, D20A_CAST_SPELL ):
            # saving throw successful
            t.float_mesfile_line( 'mes\\spell.mes', 30001 )

        else:
            # saving throw unsuccessful
            t.float_mesfile_line( 'mes\\spell.mes', 30002 )
            # HTN - apply condition HALT (Petrified)
            t.condition_add_with_args( 'sp-Command', spell.id, spell.duration, 4 )
            game.particles( 'sp-Bestow Curse', t )
            

    elif effect == 6:
        print "Prismatic Spray Effect - Insane"
        
        dc = spell.dc
        
		#insane
        if not t.saving_throw_spell( dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
            t.float_mesfile_line( 'mes\\spell.mes', 30002 )
            t.condition_add_with_args( 'sp-Confusion', spell.id, spell.duration, 1 )
        else:
            t.float_mesfile_line( 'mes\\spell.mes', 30001 )

