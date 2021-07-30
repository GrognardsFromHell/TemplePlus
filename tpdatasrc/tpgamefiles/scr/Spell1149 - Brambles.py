from toee import *

def OnBeginSpellCast(spell):
    print "Brambles OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Brambles OnSpellEffect"

    spell.duration = 1 * spell.caster_level
    spellTarget = spell.target_list[0]
    bonusDamage = min(spell.caster_level, 10)
    mainhandWeapon = spellTarget.obj.item_worn_at(item_wear_weapon_primary)

    if mainhandWeapon == OBJ_HANDLE_NULL:
        isWoodenMeleeWeapen = False
    elif mainhandWeapon.obj_get_int(obj_f_weapon_flags) & OWF_RANGED_WEAPON:
        isWoodenMeleeWeapen = False
    elif mainhandWeapon.obj_get_int(obj_f_material) == mat_wood:
        isWoodenMeleeWeapen = True
    else:
        isWoodenMeleeWeapen = False

    if isWoodenMeleeWeapen:
        mainhandWeapon.d20_status_init()
        if not mainhandWeapon.condition_add_with_args('sp-Brambles', spell.id, spell.duration, bonusDamage, 0):
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
    else:
        spellTarget.obj.float_text_line("Not a wooden melee weapon", tf_red)
        game.particles('Fizzle', spellTarget.obj)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Brambles OnBeginRound"

def OnEndSpellCast(spell):
    print "Brambles OnEndSpellCast"

