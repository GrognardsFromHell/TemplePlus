from toee import *

def OnBeginSpellCast(spell):
    print("Dispel Magic OnBeginSpellCast")
    print("spell.target_list=", spell.target_list)
    print("spell.caster=", spell.caster, " caster.level= ", spell.caster_level)

def OnSpellEffect(spell):
    print("Dispel Magic OnSpellEffect")

    targetsToRemove = []
    spell.duration = 0

    # check if we are targetting an object or an area
    if spell.is_object_selected() == 1:
        print("Single target dispel")
        isAoe = False
        spellTarget = spell.target_list[0]

        # support dispel on critters
        if (spellTarget.obj.type == obj_t_pc) or (spellTarget.obj.type == obj_t_npc):
            spellTarget.partsys_id = game.particles("sp-Dispel Magic - Targeted", spellTarget.obj)
            spellTarget.obj.condition_add_with_args("sp-Dispel Magic", spell.id, spell.duration, isAoe)

        # support dispel on portals and containers
        elif (spellTarget.obj.type == obj_t_portal) or (spellTarget.obj.type == obj_t_container):
            if spellTarget.obj.portal_flags_get() & OPF_MAGICALLY_HELD:
                spellTarget.partsys_id = game.particles("sp-Dispel Magic - Targeted", spellTarget.obj)
                spellTarget.obj.portal_flag_unset(OPF_MAGICALLY_HELD)

        # support dispel on these obj_types: weapon, ammo, armor, scroll
        # NO support for: money, food, key, written, generic, scenery, trap, bag
        #elif (spellTarget.obj.type == obj_t_weapon) or (spellTarget.obj.type == obj_t_ammo) or (spellTarget.obj.type == obj_t_armor) or (spellTarget.obj.type == obj_t_scroll):
            #print "[dispel magic] - items not supported yet!"
            #game.particles( 'Fizzle', spellTarget.obj )
            #spell.target_list.remove_target( spellTarget.obj )

        # Remove all targets in target_list to end spell properly
        for spellTarget in spell.target_list:
            targetsToRemove.append(spellTarget.obj)

    else:
        print("AoE Dispel")
        isAoe = True
        # draw area effect particles
        game.particles("sp-Dispel Magic - Area", spell.target_loc)

        for spellTarget in spell.target_list:

            if (spellTarget.obj.type == obj_t_pc) or (spellTarget.obj.type == obj_t_npc):
                spellTarget.partsys_id = game.particles("sp-Dispel Magic - Targeted", spellTarget.obj )
                spellTarget.obj.condition_add_with_args("sp-Dispel Magic", spell.id, spell.duration, isAoe)

            # support dispel on portals and containers
            elif (spellTarget.obj.type == obj_t_portal) or (spellTarget.obj.type == obj_t_container):
                if spellTarget.obj.portal_flags_get() & OPF_MAGICALLY_HELD:
                    spellTarget.partsys_id = game.particles("sp-Dispel Magic - Targeted", spellTarget.obj)
                    spellTarget.obj.portal_flag_unset(OPF_MAGICALLY_HELD)
            targetsToRemove.append(spellTarget.obj)

            # support dispel on these obj_types: weapon, ammo, armor, scroll
            # NO support for: money, food, key, written, generic, scenery, trap, bag
            #elif (spellTarget.obj.type == obj_t_weapon) or (spellTarget.obj.type == obj_t_ammo) or (spellTarget.obj.type == obj_t_armor) or (spellTarget.obj.type == obj_t_scroll):
                #print "[dispel magic] - items not supported yet!"
                #game.particles( 'Fizzle', spellTarget.obj )
                #spell.target_list.remove_target( spellTarget.obj )

    if targetsToRemove:
        spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print("Dispel Magic OnBeginRound")

def OnEndSpellCast(spell):
    print("Dispel Magic OnEndSpellCast")