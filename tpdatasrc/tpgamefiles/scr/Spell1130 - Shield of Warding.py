from toee import *

def OnBeginSpellCast(spell):
    print "Shield of Warding OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Shield of Warding OnSpellEffect"
    
    spell.duration = 10 * spell.caster_level # 1 min/cl
    spellTarget = spell.target_list[0]
    bonusValue = min ((1 + (spell.caster_level/5)), 5)
    shieldItem = spellTarget.obj.item_worn_at(item_wear_shield)

    #check if a shield is equipped
    if shieldItem.obj_get_int(obj_f_type) == obj_t_armor:
        shieldItem.d20_status_init()
        if not shieldItem.condition_add_with_args('sp-Shield of Warding', spell.id, spell.duration, bonusValue, 0):
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
    else:
        spellTarget.obj.float_text_line("Shield required", tf_red)
        game.particles('Fizzle', spellTarget.obj)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Shield of Warding OnBeginRound"

def OnEndSpellCast(spell):
    print "Shield of Warding OnEndSpellCast"