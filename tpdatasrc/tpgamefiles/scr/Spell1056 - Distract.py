from toee import *

def OnBeginSpellCast(spell):
    print "Distract OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Distract OnSpellEffect"
    
    targetsToRemove = []
    spell.duration = 1 * spell.caster_level #1 round/cl
    game.particles('sp-Sound Burst', spell.caster)

    for spellTarget in spell.target_list:
        if spellTarget.obj.hit_dice_num > 6: #Only creatures with HD 6 or less are affected
            spellTarget.obj.float_text_line("Unaffected", tf_red)
            game.particles('Fizzle', spellTarget.obj)
            targetsToRemove.append(spellTarget.obj)
        else: #Saving Throw to negate
            if spellTarget.obj.saving_throw_spell(spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id): #success
                spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
                game.particles('Fizzle', spellTarget.obj)
                targetsToRemove.append(spellTarget.obj)
            else:
                spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
                spellTarget.obj.condition_add_with_args('sp-distract', spell.id, spell.duration, 0)
                spellTarget.partsys_id = game.particles('sp-scare', spellTarget.obj)

    spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

    
def OnBeginRound(spell):
    print "Distract OnBeginRound"

def OnEndSpellCast(spell):
    print "Distract OnEndSpellCast"