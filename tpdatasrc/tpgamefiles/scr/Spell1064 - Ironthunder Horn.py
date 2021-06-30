from toee import *

def OnBeginSpellCast(spell):
    print "Ironthunder Horn OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Ironthunder Horn OnSpellEffect"

    targetsToRemove = []
    spell.duration = 0
    game.particles('sp-Ironthunder Horn', spell.caster)

    for spellTarget in spell.target_list:
        #Saving Throw to negate
        if spellTarget.obj.saving_throw_spell(spell.dc, D20_Save_Reflex, D20STD_F_NONE, spell.caster, spell.id): #success
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
            game.particles('Fizzle', spellTarget.obj)
        else:
            spellTarget.partsys_id = game.particles('sp-Shout-Hit', spellTarget.obj)
            spellTarget.obj.fall_down()
            spellTarget.obj.condition_add("Prone")
            spellTarget.obj.float_mesfile_line('mes\\combat.mes', 18, 1) #ID18: Knockdown message
        targetsToRemove.append(spellTarget.obj)
    
    spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Ironthunder Horn OnBeginRound"

def OnEndSpellCast(spell):
    print "Ironthunder Horn OnEndSpellCast"