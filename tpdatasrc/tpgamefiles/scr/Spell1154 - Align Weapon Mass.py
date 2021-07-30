from toee import *

def OnBeginSpellCast(spell):
    print "Align Weapon Mass OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Align Weapon Mass OnSpellEffect"

    targetsToRemove = []
    spell.duration = 10 * spell.caster_level
    spellCasterAlignment = spell.caster.critter_get_alignment()
    alignType = spell.spell_get_menu_arg(RADIAL_MENU_PARAM_MIN_SETTING) # 1 = Good; 2 = Evil; 3 = Lawful; 4 = Chaotic
    if not alignType in range(1,5): #Fallback
        alignType = 1 #sets it to good in fallback; alignType = game.random_range(1,4) would be a different option if a random type is prefered as a fallback
    
    if alignType == 1 and spellCasterAlignment & ALIGNMENT_EVIL:
        passedAlignmentCheck = False
    elif alignType == 2 and spellCasterAlignment & ALIGNMENT_GOOD:
        passedAlignmentCheck = False
    elif alignType == 3 and spellCasterAlignment & ALIGNMENT_CHAOTIC:
        passedAlignmentCheck = False
    elif alignType == 4 and spellCasterAlignment & ALIGNMENT_LAWFUL:
        passedAlignmentCheck = False
    else:
        passedAlignmentCheck = True
    
    if passedAlignmentCheck:
        for spellTarget in spell.target_list:
            mainhandWeapon = spellTarget.obj.item_worn_at(item_wear_weapon_primary)
            if mainhandWeapon.obj_get_int(obj_f_type) == obj_t_weapon:
                mainhandWeapon.d20_status_init()
                if not mainhandWeapon.condition_add_with_args('sp-Align Weapon', spell.id, spell.duration, alignType, 0):
                    spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
                    game.particles('Fizzle', spellTarget.obj)
            else:
                spellTarget.obj.float_text_line("Weapon required", tf_red)
                game.particles('Fizzle', spellTarget.obj)
            targetsToRemove.append(spellTarget.obj)
    else:
        for spellTarget in spell.target_list:
            targetsToRemove.append(spellTarget.obj)
        spell.caster.float_text_line("Wrong Alignment", tf_red)
        game.particles('Fizzle', spell.caster)

    spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Align Weapon Mass OnBeginRound"

def OnEndSpellCast(spell):
    print "Align Weapon Mass OnEndSpellCast"

