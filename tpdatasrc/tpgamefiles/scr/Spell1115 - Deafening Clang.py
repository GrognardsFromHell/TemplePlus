from toee import *

def OnBeginSpellCast(spell):
    print "Deafening Clang OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Deafening Clang OnSpellEffect"
    
    spell.duration = 0 #current round
    spellTarget = spell.target_list[0]
    mainhandWeapon = spellTarget.obj.item_worn_at(item_wear_weapon_primary)

    if mainhandWeapon.obj_get_int(obj_f_type) == obj_t_weapon:
        mainhandWeapon.d20_status_init()
        mainhandWeapon.condition_add_with_args('sp-Deafening Clang', spell.id, spell.duration, spell.dc, 0)
        spellTarget.partsys_id = game.particles('sp-Sound Burst', spellTarget.obj)
    else:
        spell.caster.float_text_line("Weapon required", tf_red)
        game.particles('Fizzle', spell.caster)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)


def OnBeginRound(spell):
    print "Deafening Clang OnBeginRound"

def OnEndSpellCast(spell):
    print "Deafening Clang OnEndSpellCast"