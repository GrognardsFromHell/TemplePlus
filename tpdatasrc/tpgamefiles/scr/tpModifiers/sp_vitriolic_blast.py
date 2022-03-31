from toee import *
import tpdp
import tpactions
from warlock import EldritchBlastEssenceModifier, verifyEldritchBlastAction, EldritchBlastSecondaryEffect

print "Registering sp-Vitriolic Blast"

def secondaryEffect(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    spellId = currentSequence.spell_action.spell_id
    spellPacket = tpdp.SpellPacket(spellId)
    if verifyEldritchBlastAction(spellPacket.spell_enum):
        spellTarget = evt_obj.attack_packet.target
        duration = int(spellPacket.caster_level / 5)
        stanceEnum = args.get_arg(0)
        spellTarget.condition_add_with_args("Vitriolic DoT", spellId, duration, stanceEnum, 0)
    return 0

vitriolicBlast = EldritchBlastEssenceModifier("Vitriolic Blast") #spellEnum, particlesId, empty
vitriolicBlast.AddHook(ET_OnDealingDamage2, EK_NONE, secondaryEffect, ())
vitriolicBlast.ModifyDamageType(D20DT_ACID)
vitriolicBlast.AddQuerySecondaryTrue()

### Secondary DoT Effect ###

def damageOverTime(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    spellDamageDice = dice_new("1d6")
    spellDamageDice.number = 2
    game.create_history_freeform("{} takes ~Vitriolic Blast~[TAG_SPELLS_VITRIOLIC_BLAST] acid damage\n\n".format(attachee.description))
    attachee.spell_damage(spellPacket.caster, D20DT_ACID, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spellId)
    return 0

vitriolicEffect = EldritchBlastSecondaryEffect("Vitriolic DoT") #spellId, duration, secondaryEffectEnum, empty
vitriolicEffect.AddHook(ET_OnBeginRound, EK_NONE, damageOverTime, ())
vitriolicEffect.AddSpellNoDuplicate()
