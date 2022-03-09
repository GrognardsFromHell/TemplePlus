from toee import *

def OnBeginSpellCast(spell):
    print "Soulreaving Aura OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Soulreaving Aura OnSpellEffect"

    targetsToRemove = []
    spell.duration = 0
    spellDamageDice = dice_new("1d1")
    damageType = D20DT_MAGIC
    tempHpAmount = 0

    game.particles("sp-Soulreaving Aura", spell.caster)

    for spellTarget in spell.target_list:
        if spellTarget.obj.stat_level_get(stat_hp_current) <= 0:
            spellTarget.obj.spell_damage(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id)
        targetsToRemove.append(spellTarget.obj)
        if spellTarget.obj.d20_query(Q_Dead):
            drainTempHp = min(spellTarget.obj.hit_dice_num, 10)
            tempHpAmount += drainTempHp

    if tempHpAmount:
        spell.caster.condition_add_with_args("sp-Soulreaving Aura", spell.id, spell.duration, tempHpAmount, 0)

    if targetsToRemove:
        spell.target_list.remove_list(targetsToRemove)
    if not spell.target_list:
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Soulreaving Aura OnBeginRound"

def OnEndSpellCast(spell):
    print "Soulreaving Aura OnEndSpellCast"

