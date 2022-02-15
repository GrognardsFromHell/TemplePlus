from toee import *

def OnBeginSpellCast(spell):
    print "Dirge of Discord OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Dirge of Discord OnSpellEffect"
    
    targetsToRemove = []
    spell.duration = 1 * spell.caster_level # 1 round/cl

    game.particles('sp-Sound Burst', spell.caster)

    for spellTarget in spell.target_list:
        #Save to negate spell
        if spellTarget.obj.saving_throw_spell(spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id): #success
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
            game.particles('Fizzle', spellTarget.obj)
            targetsToRemove.append(spellTarget.obj)
        else:
            spellTarget.partsys_id = game.particles('sp-Shout-Hit', spellTarget.obj)
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
            spellTarget.obj.condition_add_with_args('sp-Dirge of Discord', spell.id, spell.duration, spell.dc, 0)

    spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

    
def OnBeginRound(spell):
    print "Dirge of Discord OnBeginRound"

def OnEndSpellCast(spell):
    print "Dirge of Discord OnEndSpellCast"