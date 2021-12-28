from toee import *

def OnBeginSpellCast(spell):
    print "Bless Weapon Swift OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Bless Weapon Swift OnSpellEffect"

    spell.duration = 0
    spellTarget = spell.target_list[0]
    mainhandWeapon = spellTarget.obj.item_worn_at(item_wear_weapon_primary)
    
    if mainhandWeapon.obj_get_int(obj_f_type) == obj_t_weapon:
        mainhandWeapon.d20_status_init()
        if not mainhandWeapon.condition_add_with_args('sp-Bless Weapon', spell.id, spell.duration, 0):
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
    else:
        spellTarget.obj.float_text_line("Weapon required", tf_red)
        game.particles('Fizzle', spellTarget.obj)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Bless Weapon Swift OnBeginRound"

def OnEndSpellCast(spell):
    print "Bless Weapon Swift OnEndSpellCast"

