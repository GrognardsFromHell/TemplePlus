from toee import *

def OnBeginSpellCast(spell):
    print "Weapon of Energy OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Weapon of Energy OnSpellEffect"

    spell.duration = 1 * spell.caster_level
    spellTarget = spell.target_list[0]
    radialChoice = spell.spell_get_menu_arg(RADIAL_MENU_PARAM_MIN_SETTING) #1 = Acid, 2 = Cold, 3 = Electricity, 4 = Fire
    mainhandWeapon = spellTarget.obj.item_worn_at(item_wear_weapon_primary)

    #Fallback for radialChoice
    if not radialChoice in range(1,5):
        radialChoice = game.random_range(1,4)

    #Check if mainhandWeapon is actually a weapon
    if mainhandWeapon.obj_get_int(obj_f_type) == obj_t_weapon:
        mainhandWeapon.d20_status_init()
        if not mainhandWeapon.condition_add_with_args('sp-Weapon of Energy', spell.id, spell.duration, radialChoice, 0):
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
    else:
        spellTarget.obj.float_text_line("Weapon required", tf_red)
        game.particles('Fizzle', spellTarget.obj)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Weapon of Energy OnBeginRound"

def OnEndSpellCast(spell):
    print "Weapon of Energy OnEndSpellCast"

