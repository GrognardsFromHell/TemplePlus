from toee import *
import tpdp
from spell_utils import AoeSpellHandleModifier, AoeSpellEffectModifier, applyBonus, getSpellTargets

print "Registering sp-Anarchic Storm"

def hitRandomOutsiderinAoe(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    spellTargetList = getSpellTargets(spellPacket)
    spellDamageDice = dice_new("1d6")
    spellDamageDice.number = 5

    lawfulOutsiderInAoe = []
    for spellTarget in spellTargetList:
        alignment = spellTarget.stat_level_get(stat_alignment)
        isOutsider = True if spellTarget.is_category_type(mc_type_outsider) else False
        if alignment & ALIGNMENT_LAWFUL and isOutsider:
            lawfulOutsiderInAoe.append(spellTarget)

    if lawfulOutsiderInAoe:
        numberOfTargets = len(lawfulOutsiderInAoe)
        if numberOfTargets == 1:
            selectTarget = 0
        else:
            selectTarget = game.random_range(0, (numberOfTargets - 1))
        spellTarget = lawfulOutsiderInAoe[selectTarget]
        game.particles('sp-Axiomatic Storm-hit', spellTarget)
        game.create_history_freeform("{} is affected by ~Anarchic Storm~[TAG_SPELLS_ANARCHIC_STORM] burst\n\n".format(spellTarget.description))
        spellTarget.float_text_line("Anarchic Storm burst", tf_red)
        spellTarget.spell_damage(spellPacket.caster, D20DT_ELECTRICITY, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spellId)
    return 0

anarchicStormSpell = AoeSpellHandleModifier("sp-Anarchic Storm") #spellId, duration, bonusValue, spellEventId, spellDc, empty
anarchicStormSpell.AddHook(ET_OnBeginRound, EK_NONE, hitRandomOutsiderinAoe, ())

### Begin Anarchic Storm Effect ###

def damageOnBeginRound(attachee, args, evt_obj):
    spellTarget = attachee
    if spellTarget.stat_level_get(stat_alignment) & ALIGNMENT_LAWFUL:
        spellId = args.get_arg(0)
        spellPacket = tpdp.SpellPacket(spellId)
        spellDamageDice = dice_new("1d6")
        spellDamageDice = 4 if spellTarget.is_category_type(mc_type_outsider) else 2
        game.create_history_freeform("{} is affected by ~Anarchic Storm~[TAG_SPELLS_ANARCHIC_STORM]\n\n".format(spellTarget.description))
        spellTarget.float_text_line("Anarchic Storm", tf_red)
        spellTarget.spell_damage(spellPacket.caster, D20DT_MAGIC, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spellId)
    return 0

def stormAttackPenalty(attachee, args, evt_obj):
    if evt_obj.attack_packet.get_flags() & D20CAF_RANGED:
        applyBonus(attachee, args, evt_obj)
    return 0

stormPenalty = -4

anarchicStormEffect = AoeSpellEffectModifier("Anarchic Storm") #spellId, duration, bonusValue, spellEventId, spellDc, empty
anarchicStormEffect.AddHook(ET_OnBeginRound, EK_NONE, damageOnBeginRound, ())
anarchicStormEffect.AddSkillBonus(stormPenalty, bonus_type_storm_spell, skill_listen, skill_search, skill_spot)
anarchicStormEffect.AddHook(ET_OnToHitBonus2, EK_NONE, stormAttackPenalty, (stormPenalty, bonus_type_storm_spell,))
anarchicStormEffect.AddHook(ET_OnToHitBonusFromDefenderCondition, EK_NONE, stormAttackPenalty, (stormPenalty, bonus_type_storm_spell,))