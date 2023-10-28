from toee import *
from heritage_feat_utils import getDraconicHeritageElement

def OnBeginSpellCast(spell):
    print "Dragon Disciple Cone Breath OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Dragon Disciple Cone Breath OnSpellEffect"

    targetsToRemove = []
    spell.duration = 0
    spell.dc = 10 + spell.caster_level + ((spell.caster.stat_level_get(stat_constitution)- 10 ) / 2)

    spellDamageDice = dice_new('1d8')
    if spell.caster_level < 7:
        spellDamageDice.number = 2
    elif spell.caster_level < 10:
        spellDamageDice.number = 4
    else:
        spellDamageDice.number = 6
    saveType = D20_Save_Reduction_Half
    
    heritage = spell.caster.d20_query("PQ_Selected_Draconic_Heritage")
    damageType = getDraconicHeritageElement(heritage)
    if damageType == D20DT_ACID:
        saveDescriptor = D20STD_F_SPELL_DESCRIPTOR_ACID
        elementString = "Acid"
    elif damageType == D20DT_COLD:
        saveDescriptor = D20STD_F_SPELL_DESCRIPTOR_COLD
        elementString = "Cold"
    elif damageType == D20DT_ELECTRICITY:
        saveDescriptor = D20STD_F_SPELL_DESCRIPTOR_ELECTRICITY
        elementString = "Electricity"
    elif damageType == D20DT_FIRE:
        saveDescriptor = D20STD_F_SPELL_DESCRIPTOR_FIRE
        elementString = "Fire"
    else: #Fallback
        saveDescriptor = D20STD_F_NONE
        elementString = "Fire"
    particleEffect = "sp-Breath Weapon Cone Medium {}".format(elementString)

    game.particles(particleEffect, spell.caster)

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
    print "Dragon Disciple Cone Breath OnBeginRound"

def OnEndSpellCast(spell):
    print "Dragon Disciple Cone Breath OnEndSpellCast"