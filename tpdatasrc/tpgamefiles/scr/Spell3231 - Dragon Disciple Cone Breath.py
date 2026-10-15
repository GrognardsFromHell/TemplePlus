from toee import *
# Dragon Disciple breath weapon: 30 ft cone, Reflex half, DC 10 + DD level + Con mod.
# Damage 2d8 (DD 3-6), 4d8 (DD 7-9), 6d8 (DD 10). Energy type from the draconic heritage.
# Cone / save handling follows Spell1151 - Frost Breath.

ENERGY = {1: D20DT_ACID, 2: D20DT_ELECTRICITY, 3: D20DT_ACID, 4: D20DT_FIRE, 5: D20DT_COLD,
          6: D20DT_FIRE, 7: D20DT_ELECTRICITY, 8: D20DT_ACID, 9: D20DT_FIRE, 10: D20DT_COLD}
DESCRIPTOR = {D20DT_ACID: D20STD_F_SPELL_DESCRIPTOR_ACID, D20DT_ELECTRICITY: D20STD_F_SPELL_DESCRIPTOR_ELECTRICITY,
              D20DT_FIRE: D20STD_F_SPELL_DESCRIPTOR_FIRE, D20DT_COLD: D20STD_F_SPELL_DESCRIPTOR_COLD}
PARTICLES = {D20DT_FIRE: 'sp-Burning Hands', D20DT_COLD: 'sp-Cone of Cold'}

def OnBeginSpellCast(spell):
    print "Dragon Disciple Cone Breath OnBeginSpellCast"

def OnSpellEffect(spell):
    print "Dragon Disciple Cone Breath OnSpellEffect"
    caster = spell.caster
    ddLevel = caster.stat_level_get(stat_level_dragon_disciple)
    spell.duration = 0
    spell.dc = 10 + ddLevel + ((caster.stat_level_get(stat_constitution) - 10) / 2)

    damageDice = dice_new('1d8')
    if ddLevel >= 10:
        damageDice.number = 6
    elif ddLevel >= 7:
        damageDice.number = 4
    else:
        damageDice.number = 2

    heritage = caster.d20_query("PQ_Dragon_Disciple_Heritage")
    damageType = ENERGY.get(heritage, D20DT_FIRE)
    saveDescriptor = DESCRIPTOR.get(damageType, D20STD_F_NONE)
    game.particles(PARTICLES.get(damageType, 'sp-Frost Breath'), caster)

    targetsToRemove = []
    for spellTarget in spell.target_list:
        tgt = spellTarget.obj
        if tgt.saving_throw_spell(spell.dc, D20_Save_Reflex, saveDescriptor, caster, spell.id): # success: half damage
            tgt.float_mesfile_line('mes\\spell.mes', 30001)
            tgt.spell_damage_with_reduction(caster, damageType, damageDice, D20DAP_UNSPECIFIED, DAMAGE_REDUCTION_HALF, D20A_CAST_SPELL, spell.id)
        else:
            tgt.float_mesfile_line('mes\\spell.mes', 30002)
            tgt.spell_damage(caster, damageType, damageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id)
        targetsToRemove.append(tgt)

    if targetsToRemove:
        spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Dragon Disciple Cone Breath OnBeginRound"

def OnEndSpellCast(spell):
    print "Dragon Disciple Cone Breath OnEndSpellCast"
