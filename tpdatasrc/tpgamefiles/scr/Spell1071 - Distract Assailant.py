from toee import *

def OnBeginSpellCast(spell):
    print "Distract Assailant OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Distract Assailant OnSpellEffect"
    
    spell.duration = 0 #Spell Target is considered flatfooted until beginning of its next turn.
    spellTarget = spell.target_list[0]

    #Saving Throw to negate
    if spellTarget.obj.saving_throw_spell(spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id): #success
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
        if spellTarget.obj.condition_add_with_args('sp-Distract Assailant', spell.id, spell.duration, 0):
            spellTarget.partsys_id = game.particles('sp-Scare', spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
            spell.target_list.remove_target(spellTarget.obj)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Distract Assailant OnBeginRound"

def OnEndSpellCast(spell):
    print "Distract Assailant OnEndSpellCast"