from toee import *

def OnBeginSpellCast( spell ):
    print "Warlock Charm OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect( spell ):
    print "Warlock Charm OnSpellEffect"

    spell.duration = 14400 * spell.caster_level
    spellTarget = spell.target_list[0]
    saveType = D20_Save_Will
    saveDescriptor = D20STD_F_NONE

    if spellTarget.obj.is_friendly(spell.caster):
        game.particles("Fizzle", spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
    else: #Saving Throw to avoid effect
        if spellTarget.obj.saving_throw_spell(spell.dc, saveType, saveDescriptor, spell.caster, spell.id): #success
            spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30001)
            game.particles("Fizzle", spellTarget.obj)
            spell.target_list.remove_target(spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30002)
            spell.caster.ai_follower_add(spellTarget.obj)
            spellTarget.obj.condition_add_with_args("sp-Charm Monster", spell.id, spell.duration, spellTarget.obj.hit_dice_num)
            spellTarget.partsys_id = game.particles("sp-Charm Monster", spellTarget.obj)
            #Add target to initiative, just in case
            spellTarget.obj.add_to_initiative()
            game.update_combat_ui()
    spell.spell_end(spell.id)

def OnBeginRound( spell ):
    print "Warlock Charm OnBeginRound"

def OnEndSpellCast( spell ):
    print "Warlock Charm OnEndSpellCast"