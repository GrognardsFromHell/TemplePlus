from toee import *

def OnBeginSpellCast(spell):
    print "Heralds Call OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Heralds Call OnSpellEffect"
    
    targetsToRemove = []
    spell.duration = 1 #1 round

    game.particles('sp-Sound Burst', spell.caster)

    for spellTarget in spell.target_list:
        if spellTarget.obj.hit_dice_num > 5: #Only creatures with HD 5 or less are affected
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
                if spellTarget.obj.condition_add_with_args('sp-slow', spell.id, spell.duration, 0):
                    spellTarget.partsys_id = game.particles('sp-slow', spellTarget.obj)
                else:
                    spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
                    game.particles('Fizzle', spellTarget.obj)
                    targetsToRemove.append(spellTarget.obj)

    if targetsToRemove:
        spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

    
def OnBeginRound(spell):
    print "Heralds Call OnBeginRound"

def OnEndSpellCast(spell):
    print "Heralds Call OnEndSpellCast"