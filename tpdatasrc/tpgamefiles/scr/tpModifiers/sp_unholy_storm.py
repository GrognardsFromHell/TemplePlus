from toee import *
import tpdp
from spell_utils import AoeSpellHandleModifier, AoeSpellEffectModifier, applyBonus

print "Registering sp-Unholy Storm"

unholyStormSpell = AoeSpellHandleModifier("sp-Unholy Storm") #spellId, duration, bonusValue, spellEventId, spellDc, empty

### Begin Unholy Storm Effect ###

def damageOnBeginRound(attachee, args, evt_obj):
    spellTarget = attachee
    if spellTarget.stat_level_get(stat_alignment) & ALIGNMENT_EVIL:
        spellId = args.get_arg(0)
        spellPacket = tpdp.SpellPacket(spellId)
        spellDamageDice = dice_new("1d6")
        spellDamageDice = 4 if spellTarget.is_category_type(mc_type_outsider) else 2
        game.create_history_freeform("{} is affected by ~Unholy Storm~[TAG_SPELLS_UNHOLY_STORM]\n\n".format(spellTarget.description))
        spellTarget.float_text_line("Unholy Storm", tf_red)
        spellTarget.spell_damage(spellPacket.caster, D20DT_MAGIC, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spellId)
    return 0

def stormAttackPenalty(attachee, args, evt_obj):
    if evt_obj.attack_packet.get_flags() & D20CAF_RANGED:
        applyBonus(attachee, args, evt_obj)
    return 0

stormPenalty = -4

unholyStormEffect = AoeSpellEffectModifier("Unholy Storm") #spellId, duration, bonusValue, spellEventId, spellDc, empty
unholyStormEffect.AddHook(ET_OnBeginRound, EK_NONE, damageOnBeginRound, ())
unholyStormEffect.AddSkillBonus(stormPenalty, bonus_type_storm_spell, skill_listen, skill_search, skill_spot)
unholyStormEffect.AddHook(ET_OnToHitBonus2, EK_NONE, stormAttackPenalty, (stormPenalty, bonus_type_storm_spell,))
unholyStormEffect.AddHook(ET_OnToHitBonusFromDefenderCondition, EK_NONE, stormAttackPenalty, (stormPenalty, bonus_type_storm_spell,))