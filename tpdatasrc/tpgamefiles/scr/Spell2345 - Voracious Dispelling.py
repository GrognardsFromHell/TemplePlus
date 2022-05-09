from toee import *
import tpdp

def OnBeginSpellCast(spell):
    print("Voracious Dispelling OnBeginSpellCast")
    print("spell.target_list=", spell.target_list)
    print("spell.caster=", spell.caster, " caster.level= ", spell.caster_level)

def OnSpellEffect(spell):
    print("Voracious Dispelling OnSpellEffect")
    
    targetsToRemove = []
    spell.duration = 0

    # Check target mode
    if spell.is_object_selected() == 1:
        print("Single target dispel")
        isAoe = False
        spellTarget = spell.target_list[0]
        if spellTarget.obj.type == obj_t_pc or spellTarget.obj.type == obj_t_npc:
            spellTarget.partsys_id = game.particles("sp-Dispel Magic - Targeted", spellTarget.obj)
            spellTarget.obj.condition_add_with_args("sp-Voracious Dispelling", spell.id, spell.duration, isAoe, 0)
        elif spellTarget.obj.type == obj_t_portal or spellTarget.obj.type == obj_t_container:
            if spellTarget.obj.portal_flags_get() & OPF_MAGICALLY_HELD:
                spellTarget.partsys_id = game.particles("sp-Dispel Magic - Targeted", spellTarget.obj)
                spellTarget.obj.portal_flag_unset(OPF_MAGICALLY_HELD)
            else:
                spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30000)
                game.particles("Fizzle", spellTarget.obj)
        # Remove all targets in target_list to end spell properly
        for spellTarget in spell.target_list:
            targetsToRemove.append(spellTarget.obj)

    else:
        print("AoE Dispel")
        isAoe = True
        game.particles("sp-Dispel Magic - Area", spell.target_loc)
        for spellTarget in spell.target_list:
            if spellTarget.obj.type == obj_t_pc or spellTarget.obj.type == obj_t_npc:
                spellTarget.obj.condition_add_with_args("sp-Voracious Dispelling", spell.id, spell.duration, isAoe, 0)
            elif spellTarget.obj.type == obj_t_portal or spellTarget.obj.type == obj_t_container:
                if spellTarget.obj.portal_flags_get() & OPF_MAGICALLY_HELD:
                    spellTarget.obj.portal_flag_unset(OPF_MAGICALLY_HELD)
                else:
                    spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30000)
                    game.particles("Fizzle", spellTarget.obj)
            targetsToRemove.append(spellTarget.obj)

    if targetsToRemove:
        spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print("Voracious Dispelling OnBeginRound")

def OnEndSpellCast(spell):
    print("Voracious Dispelling OnEndSpellCast")

