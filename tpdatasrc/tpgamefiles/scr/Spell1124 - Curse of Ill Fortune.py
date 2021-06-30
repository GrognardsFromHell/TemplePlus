from toee import *

def OnBeginSpellCast(spell):
    print "Curse of Ill Fortune OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Curse of Ill Fortune OnSpellEffect"
    
    spell.duration = 10 * spell.caster_level #1 minute/casterlevel
    spellTarget = spell.target_list[0]
    
    if spellTarget.obj.is_category_type(mc_type_construct) or spellTarget.obj.is_category_type(mc_type_undead): # curse affects only living targets
        spellTarget.obj.float_text_line("Not a living creature")
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
    else:
        #Saving Throw to negate
        if spellTarget.obj.saving_throw_spell(spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id): #success
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
            game.particles('Fizzle', spellTarget.obj)
            spell.target_list.remove_target(spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
            spellTarget.obj.condition_add_with_args('sp-Curse of Ill Fortune', spell.id, spell.duration)
            spellTarget.partsys_id = game.particles('sp-Bestow Curse', spellTarget.obj)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Curse of Ill Fortune OnBeginRound"

def OnEndSpellCast(spell):
    print "Curse of Ill Fortune OnEndSpellCast"