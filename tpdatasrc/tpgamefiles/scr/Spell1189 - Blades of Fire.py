from toee import *

def OnBeginSpellCast(spell):
    print "Blades of Fire OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Blades of Fire OnSpellEffect"

    spell.duration = 0 # current round
    spellTarget = spell.target_list[0]
    mainhandWeapon = spellTarget.obj.item_worn_at(item_wear_weapon_primary)
    offhandWeapon = spellTarget.obj.item_worn_at(item_wear_weapon_secondary)
    actionMesId = AEC_WRONG_WEAPON_TYPE + 1000 #{1017}{Wrong Weapon Type}
    weaponTypeMainhand = mainhandWeapon.get_weapon_type()
    weaponTypeOffhand = offhandWeapon.get_weapon_type()

    if game.is_melee_weapon(weaponTypeMainhand) and mainhandWeapon != OBJ_HANDLE_NULL:
        mainhandWeapon.d20_status_init()
        if not mainhandWeapon.condition_add_with_args('sp-Blades of Fire', spell.id, spell.duration, 0):
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\action.mes', actionMesId, tf_red)
        game.particles('Fizzle', spellTarget.obj)

    if game.is_melee_weapon(weaponTypeOffhand) and offhandWeapon != OBJ_HANDLE_NULL:
        offhandWeapon.d20_status_init()
        if not offhandWeapon.condition_add_with_args('sp-Blades of Fire', spell.id, spell.duration, 0):
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
    elif offhandWeapon != OBJ_HANDLE_NULL:
        spellTarget.obj.float_mesfile_line('mes\\action.mes', actionMesId, tf_red)
        game.particles('Fizzle', spellTarget.obj)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Blades of Fire OnBeginRound"

def OnEndSpellCast(spell):
    print "Blades of Fire OnEndSpellCast"