from toee import *
from heritage_feat_utils import getDraconicHeritageElement

def OnBeginSpellCast(spell):
    print "Draconic Breath Line OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Draconic Breath Line OnSpellEffect"

    targetsToRemove = []
    spell.duration = 0
    heritage = spell.caster.d20_query("PQ_Selected_Draconic_Heritage")
    damageType = getDraconicHeritageElement(heritage)
    saveType = D20_Save_Reduction_Half
    #Spell DC is set wrong by engine, ignoring the spell_level
    spell.dc = 10 + spell.spell_level + ((spell.caster.stat_level_get(stat_charisma)-10)/2)

    if spell.caster.stat_level_get(stat_level_dragonheart_mage) in range (6, 10):
        spellDamageDice = dice_new('1d8')
    else:
        spellDamageDice = dice_new('1d6')
    if spell.caster.stat_level_get(stat_level_dragonheart_mage) >= 10:
        spellDamageDice.number = 3 * spell.spell_level
    else:
        spellDamageDice.number = 2 * spell.spell_level

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
        saveDescriptor = D20STD_F_SPELL_DESCRIPTOR_FIRE
        elementString = "Fire"
    particleEffect = "sp-Breath Weapon Line Medium {}".format(elementString)

    game.particles(particleEffect, spell.caster)

    for spellTarget in spell.target_list:
        #Save for half damage:
        if spellTarget.obj.saving_throw_spell(spell.dc, saveType, saveDescriptor, spell.caster, spell.id): #success
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
            spellTarget.obj.spell_damage_with_reduction(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, DAMAGE_REDUCTION_HALF, D20A_CAST_SPELL, spell.id)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
            spellTarget.obj.spell_damage(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id)
        targetsToRemove.append(spellTarget.obj)

    spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Draconic Breath Line OnBeginRound"

def OnEndSpellCast(spell):
    print "Draconic Breath Line OnEndSpellCast"