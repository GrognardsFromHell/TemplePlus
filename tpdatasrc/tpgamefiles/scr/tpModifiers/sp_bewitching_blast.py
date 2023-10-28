from toee import *
import tpdp
import tpactions
from warlock import EldritchBlastEssenceModifier, verifyEldritchBlastAction

print "Registering sp-Bewitching Blast"

def secondaryEffect(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    spellId = currentSequence.spell_action.spell_id
    spellPacket = tpdp.SpellPacket(spellId)
    if verifyEldritchBlastAction(spellPacket.spell_enum):
        spellDc = spellPacket.dc
        saveType = D20_Save_Will
        saveDescriptor = D20STD_F_SPELL_DESCRIPTOR_MIND_AFFECTING
        spellTarget = evt_obj.attack_packet.target
        if spellTarget.saving_throw_spell(spellDc, saveType, saveDescriptor, spellPacket.caster, spellId): #success
            spellTarget.float_mesfile_line("mes\\spell.mes", 30001)
            game.particles("Fizzle", spellTarget)
        else:
            spellTarget.float_mesfile_line("mes\\spell.mes", 30002)
            duration = 1
            spellTarget.condition_add_with_args("sp-Confusion", spellId, duration, 0)
            #sp-Confusion-Hit
    return 0

bewitchingBlast = EldritchBlastEssenceModifier("Bewitching Blast") #spellEnum, particlesId, empty
bewitchingBlast.AddHook(ET_OnDealingDamage2, EK_NONE, secondaryEffect, ())
bewitchingBlast.AddQuerySecondaryTrue()
