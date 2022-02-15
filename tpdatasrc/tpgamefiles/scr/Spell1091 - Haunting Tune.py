from toee import *

def OnBeginSpellCast(spell):
    print "Haunting Tune OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Haunting Tune OnSpellEffect"
    
    targetsToRemove = []
    spell.duration = 100 * spell.caster_level # 10 min/cl

    game.particles('sp-Sound Burst', spell.caster)

    for spellTarget in spell.target_list:
        if spellTarget.obj.saving_throw_spell(spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id): #success
            game.particles('Fizzle', spellTarget.obj)
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
            targetsToRemove.append(spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
            spellTarget.obj.condition_add("sp-Haunting Tune", spell.duration)

    spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Haunting Tune OnBeginRound"

def OnEndSpellCast(spell):
    print "Haunting Tune OnEndSpellCast"