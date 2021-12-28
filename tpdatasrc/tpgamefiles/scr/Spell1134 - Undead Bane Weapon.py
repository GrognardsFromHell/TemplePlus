from toee import *

def OnBeginSpellCast(spell):
    print "Undead Bane Weapon OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Undead Bane Weapon OnSpellEffect"

    spell.duration = 600 * spell.caster_level # 1 hour/caster_level
    spellTarget = spell.target_list[0]
    mainhandWeapon = spellTarget.obj.item_worn_at(item_wear_weapon_primary)

    if mainhandWeapon.obj_get_int(obj_f_type) == obj_t_weapon:
        mainhandWeapon.d20_status_init()
        if not mainhandWeapon.condition_add_with_args('sp-Undead Bane Weapon', spell.id, spell.duration, 0):
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
    else:
        spellTarget.obj.float_text_line("Weapon required", tf_red)
        game.particles('Fizzle', spellTarget.obj)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Undead Bane Weapon OnBeginRound"

def OnEndSpellCast(spell):
    print "Undead Bane Weapon OnEndSpellCast"

