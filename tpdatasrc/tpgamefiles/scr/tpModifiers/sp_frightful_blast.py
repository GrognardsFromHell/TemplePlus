from toee import *
import tpdp
import tpactions
from warlock import EldritchBlastEssenceModifier, verifyEldritchBlastAction

print "Registering sp-Frightful Blast"

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
            duration = 10 #1 min
            spellTarget.condition_add_with_args("Shaken", duration, 0, 0)
    return 0

frightfulBlast = EldritchBlastEssenceModifier("sp-Frightful Blast") #spellEnum, empty
frightfulBlast.AddHook(ET_OnDealingDamage2, EK_NONE, secondaryEffect, ())