from toee import *

def OnBeginSpellCast(spell):
    print "Masters Touch OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Masters Touch OnSpellEffect"
    spell.duration = 10 * spell.caster_level # 1 Min/cl
    spellTarget = spell.target_list[0]

    itemTarget = spell.spell_get_menu_arg(RADIAL_MENU_PARAM_MIN_SETTING) # 1 = mainhand; 2 = offhand; 3 = shield
    if itemTarget == 1:
        itemTarget = item_wear_weapon_primary
    elif itemTarget == 2:
        itemTarget = item_wear_weapon_secondary
    elif itemTarget == 3:
        itemTarget = item_wear_shield
    else:
        itemTarget = item_wear_weapon_primary #sets it to mainhand in fallback

    wornItem = spellTarget.obj.item_worn_at(itemTarget)

    if itemTarget < item_wear_shield:
        if wornItem.type == obj_t_weapon:
            weaponType = wornItem.get_weapon_type()
            spellTarget.obj.condition_add_with_args("sp-Masters Touch", spell.id, spell.duration, weaponType, 0)
        else:
            spellTarget.obj.float_text_line("Weapon required!", tf_red)
            game.particles('Fizzle', spellTarget.obj)
            spell.target_list.remove_target(spellTarget.obj)
    else:
        if wornItem.type == obj_t_armor:
            spellTarget.obj.condition_add_with_args("sp-Masters Touch", spell.id, spell.duration, -1) #-1 indicates shield
        else:
            spellTarget.obj.float_text_line("Shield required", tf_red)
            game.particles('Fizzle', spellTarget.obj)
            spell.target_list.remove_target(spellTarget.obj)

    spell.spell_end( spell.id)

def OnBeginRound(spell):
    print "Masters Touch OnBeginRound"

def OnEndSpellCast(spell):
    print "Masters Touch OnEndSpellCast"