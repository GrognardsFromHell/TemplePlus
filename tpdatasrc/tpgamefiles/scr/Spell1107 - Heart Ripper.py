from toee import *
from spell_utils import checkCategoryType

def OnBeginSpellCast(spell):
    print "Heart Ripper OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Heart Ripper OnSpellEffect"

    spell.duration = 0
    spellTarget = spell.target_list[0]
    mcTypeImmunity = checkCategoryType(spellTarget.obj, mc_type_construct, mc_type_elemental, mc_type_plant, mc_type_ooze, mc_type_undead)
    critImmunity = True if spellTarget.obj.d20_query(Q_Critter_Is_Immune_Critical_Hits) else False

    game.particles('sp-Slay Living', spellTarget.obj)

    if mcTypeImmunity:
        spellTarget.obj.float_text_line("Unaffected due to Racial Immunity")
        game.particles('Fizzle', spellTarget.obj)
    elif critImmunity:
        spellTarget.obj.float_text_line("Unaffected due to Critical Hit Immunity")
        game.particles('Fizzle', spellTarget.obj)
    else:
        if spellTarget.obj.saving_throw_spell(spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id):
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
            game.particles('Fizzle', spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
            if spellTarget.obj.hit_dice_num > spell.caster_level: #If creatures HD > spell.caster_level, can't be instant killed
                durationDice = dice_new('1d4')
                stunDuration = durationDice.roll()
                spellTarget.obj.condition_add('Stunned', stunDuration)
            else:
                spellTarget.obj.critter_kill_by_effect(spell.caster) #Death Ward protects from Heart Ripper

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

    
def OnBeginRound(spell):
    print "Heart Ripper OnBeginRound"

def OnEndSpellCast(spell):
    print "Heart Ripper OnEndSpellCast"
