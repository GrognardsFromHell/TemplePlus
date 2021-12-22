from toee import *

def OnBeginSpellCast(spell):
    print "Sonic Weapon OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Sonic Weapon OnSpellEffect"
    
    spell.duration = 10 * spell.caster_level # 1 min/cl
    spellTarget = spell.target_list[0]
    radialChoice = spell.spell_get_menu_arg(RADIAL_MENU_PARAM_MIN_SETTING)
    if radialChoice == 1:
        itemToEnchant = spellTarget.obj.item_worn_at(item_wear_weapon_primary)
    elif radialChoice == 2:
        itemToEnchant = spellTarget.obj.item_worn_at(item_wear_weapon_secondary)
    elif radialChoice == 3:
        itemToEnchant = spellTarget.obj.item_worn_at(item_wear_ammo)
    else: #Fallback
        radialChoice == 1
        itemToEnchant = spellTarget.obj.item_worn_at(item_wear_weapon_primary)

    if radialChoice < 3:
        if itemToEnchant.obj_get_int(obj_f_type) == obj_t_weapon:
            itemToEnchant.d20_status_init()
            if itemToEnchant.condition_add_with_args('sp-Sonic Weapon', spell.id, spell.duration, 0):
                spellTarget.partsys_id = game.particles('sp-Sound Burst', spellTarget.obj)
            else:
                spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
                game.particles('Fizzle', spellTarget.obj)
        else:
            spellTarget.obj.float_text_line("Weapon required", tf_red)
            game.particles('Fizzle', spellTarget.obj)
    else:
        pass

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Sonic Weapon OnBeginRound"

def OnEndSpellCast(spell):
    print "Sonic Weapon OnEndSpellCast"