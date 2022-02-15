from toee import *

def OnBeginSpellCast(spell):
    print "Resonating Bolt OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect(spell):
    print "Resonating Bolt OnSpellEffect"
    
    targetsToRemove = []
    spell.duration = 0

    spellDamageDice = dice_new('1d4')
    spellDamageDice.number = min(spell.caster_level, 10) #capped at CL 10
    saveType = D20_Save_Reduction_Half
    saveDescriptor = D20STD_F_NONE
    damageType = D20DT_SONIC

    game.particles( 'sp-Lightning Bolt', spell.target_loc)
    game.pfx_lightning_bolt(spell.caster, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y, spell.target_loc_off_z)

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
    print "Resonating Bolt OnBeginRound"

def OnEndSpellCast(spell):
    print "Resonating Bolt OnEndSpellCast"