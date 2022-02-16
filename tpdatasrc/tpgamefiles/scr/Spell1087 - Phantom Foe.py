from toee import *

def OnBeginSpellCast(spell):
    print "Phantom Foe OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
    #game.particles("sp-illusion-conjure", spell.caster)

def OnSpellEffect(spell):
    print "Phantom Foe OnSpellEffect"
    
    spell.duration = 1 * spell.caster_level #1 round/casterlevel
    spellTarget = spell.target_list[0]
    
    #Saving Throw to negate
    if spellTarget.obj.saving_throw_spell(spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id): #success
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
        spellTarget.obj.condition_add_with_args('sp-Phantom Foe', spell.id, spell.duration)
        spellTarget.partsys_id = game.particles('sp-Scare', spellTarget.obj)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Phantom Foe OnBeginRound"

def OnEndSpellCast(spell):
    print "Phantom Foe OnEndSpellCast"