from toee import *

def OnBeginSpellCast(spell):
    print "Eldritch Chain OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
    print "spell.spell_level=", spell.spell_level
    print "spell.dc=", spell.dc

def OnSpellEffect(spell):
    print "Eldritch Chain OnSpellEffect"

    targetsToRemove = []
    spell.duration = 0
    spellDamageDice = dice_new("1d6")
    spellDamageDice.number = min(int(min((spell.caster_level + 1) / 2, 6)) + int(max((spell.caster_level - 11) / 3, 0)), 9) #capped at 9d6 at cl 20
    damageType = spell.caster.d20_query("PQ_Eldritch_Blast_Damage_Type")
    if not damageType:
        damageType = D20DT_MAGIC
    spellDamageReduction = 100 #100 indicates full damage

    #game.particles("sp-Ray of Enfeeblement", spell.target_loc)
    game.pfx_chain_lightning( spell.caster, spell.num_of_targets, spell.target_list)

    missedEarlierTarget = False
    secondaryTarget = False
    for spellTarget in spell.target_list:
        if missedEarlierTarget:
            spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30007)
            game.particles("Fizzle", spellTarget.obj)
            targetsToRemove.append(spellTarget.obj)
            continue
        attackResult = spell.caster.perform_touch_attack(spellTarget.obj)
        if attackResult & D20CAF_HIT:
            if secondaryTarget:
                spellDamageDice.number /= 2
            spellTarget.obj.spell_damage_weaponlike(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, spellDamageReduction, D20A_CAST_SPELL, spell.id, attackResult, 0)
            secondaryTarget = True
            if not spellTarget.obj.d20_query("PQ_Eldritch_Blast_Has_Secondary_Effect"):
                targetsToRemove.append(spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30007)
            game.particles("Fizzle", spellTarget.obj)
            missedEarlierTarget = True
            targetsToRemove.append(spellTarget.obj)
    if targetsToRemove:
        spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Eldritch Chain OnBeginRound"

def OnEndSpellCast(spell):
    print "Eldritch Chain OnEndSpellCast"

