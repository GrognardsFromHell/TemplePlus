from toee import *

def OnBeginSpellCast(spell):
    print "Blast of Flame OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Blast of Flame OnSpellEffect"

    targetsToRemove = []
    spell.duration = 0
    saveType = D20_Save_Reduction_Half
    saveDescriptor = D20STD_F_NONE
    spellDamageDice = dice_new('1d6')
    spellDamageDice.number = min(spell.caster_level, 10)
    damageType = D20DT_FIRE

    game.particles('sp-Blast of Flame', spell.caster)

    for spellTarget in spell.target_list:
        #Save for half damage
        if spellTarget.obj.reflex_save_and_damage(spell.caster, spell.dc, saveType, saveDescriptor, spellDamageDice, damageType, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id): #success
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
        targetsToRemove.append(spellTarget.obj)

    spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Blast of Flame OnBeginRound"

def OnEndSpellCast(spell):
    print "Blast of Flame OnEndSpellCast"

