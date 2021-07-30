from toee import *

def OnBeginSpellCast(spell):
    print "Lawful Sword OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Lawful Sword OnSpellEffect"

    spell.duration = 1 * spell.caster_level
    spellTarget = spell.target_list[0]
    mainhandWeapon = spellTarget.obj.item_worn_at(item_wear_weapon_primary)
    if mainhandWeapon.obj_get_int(obj_f_type) == obj_t_weapon:
        if mainhandWeapon.obj_get_int(obj_f_weapon_flags) & OWF_RANGED_WEAPON:
            spell.caster.float_text_line("Melee Weapon required", tf_red)
            game.particles('Fizzle', spell.caster)
        else:
            mainhandWeapon.d20_status_init()
            if not mainhandWeapon.condition_add_with_args('sp-Lawful Sword', spell.id, spell.duration, 0):
                spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
                game.particles('Fizzle', spellTarget.obj)
    else:
        spellTarget.obj.float_text_line("Weapon required", tf_red)
        game.particles('Fizzle', spellTarget.obj)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Lawful Sword OnBeginRound"

def OnEndSpellCast(spell):
    print "Lawful Sword OnEndSpellCast"

