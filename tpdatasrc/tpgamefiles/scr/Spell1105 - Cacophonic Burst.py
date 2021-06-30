from toee import *

def OnBeginSpellCast(spell):
    print "Cacophonic Burst OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect(spell):
    print "Cacophonic Burst OnSpellEffect"
    
    targetsToRemove = []
    spell.duration = 0

    spellDamageDice = dice_new('1d6')
    spellDamageDice.number = min(spell.caster_level, 15) #capped at CL 15
    saveType = D20_Save_Reduction_Half
    saveDescriptor = D20STD_F_NONE
    damageType = D20DT_SONIC

    game.particles( 'sp-Sound Burst', spell.target_loc )

    for spellTarget in spell.target_list:
        #Save for half damage:
        if spellTarget.obj.reflex_save_and_damage(spell.caster, spell.dc, saveType, saveDescriptor, spellDamageDice, damageType, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id): #success
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
        targetsToRemove.append(spellTarget.obj)

    spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Cacophonic Burst OnBeginRound"

def OnEndSpellCast(spell):
    print "Cacophonic Burst OnEndSpellCast"