from toee import *
import tpdp
from spell_utils import AoeSpellHandleModifier, AoeSpellEffectModifier, applyBonus

print "Registering sp-Holy Storm"

holyStormSpell = AoeSpellHandleModifier("sp-Holy Storm") #spellId, duration, bonusValue, spellEventId, spellDc, empty

### Begin Holy Storm Effect ###

def damageOnBeginRound(attachee, args, evt_obj):
    spellTarget = attachee
    if spellTarget.stat_level_get(stat_alignment) & ALIGNMENT_EVIL:
        spellId = args.get_arg(0)
        spellPacket = tpdp.SpellPacket(spellId)
        spellDamageDice = dice_new("1d6")
        spellDamageDice = 4 if spellTarget.is_category_type(mc_type_outsider) else 2
        game.create_history_freeform("{} is affected by ~Holy Storm~[TAG_SPELLS_HOLY_STORM]\n\n".format(spellTarget.description))
        spellTarget.float_text_line("Holy Storm", tf_red)
        spellTarget.spell_damage(spellPacket.caster, D20DT_MAGIC, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spellId)
    return 0

def stormAttackPenalty(attachee, args, evt_obj):
    if evt_obj.attack_packet.get_flags() & D20CAF_RANGED:
        applyBonus(attachee, args, evt_obj)
    return 0

stormPenalty = -4

holyStormEffect = AoeSpellEffectModifier("Holy Storm") #spellId, duration, bonusValue, spellEventId, spellDc, empty
holyStormEffect.AddHook(ET_OnBeginRound, EK_NONE, damageOnBeginRound, ())
holyStormEffect.AddSkillBonus(stormPenalty, bonus_type_storm_spell, skill_listen, skill_search, skill_spot)
holyStormEffect.AddHook(ET_OnToHitBonus2, EK_NONE, stormAttackPenalty, (stormPenalty, bonus_type_storm_spell,))
holyStormEffect.AddHook(ET_OnToHitBonusFromDefenderCondition, EK_NONE, stormAttackPenalty, (stormPenalty, bonus_type_storm_spell,))
