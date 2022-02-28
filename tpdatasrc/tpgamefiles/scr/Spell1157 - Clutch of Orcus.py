from toee import *

def OnBeginSpellCast(spell):
    print "Clutch of Orcus OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Clutch of Orcus OnSpellEffect"

    spell.duration = 1 * spell.caster_level
    spellTarget = spell.target_list[0]
    spellDamageDice = dice_new("1d12")

    if spellTarget.obj.is_category_type(mc_type_humanoid):
        if spellTarget.obj.saving_throw_spell(spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id):
            spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30001)
            game.particles("Fizzle", spellTarget.obj)
            spell.target_list.remove_target(spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30002)
            spellTarget.obj.spell_damage(spell.caster, D20DT_MAGIC, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id)
            spellTarget.obj.condition_add_with_args("sp-Clutch of Orcus", spell.id, spell.duration, spell.dc, 0)
            spellTarget.obj.condition_add_with_args("Paralyzed", spell.duration)
            spellTarget.partsys_id = game.particles("sp-Ghoul Touch", spellTarget.obj)
    else:
        spellTarget.obj.float_text_line("Unaffected due to Racial Immunity")
        game.particles("Fizzle", spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Clutch of Orcus OnBeginRound"

def OnEndSpellCast(spell):
    print "Clutch of Orcus OnEndSpellCast"

