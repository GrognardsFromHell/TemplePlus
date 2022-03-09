from toee import *
import tpdp
import tpactions
from warlock import EldritchBlastEssenceModifier, verifyEldritchBlastAction
from spell_utils import SpellPythonModifier

print "Registering sp-Hellrime Blast"

def secondaryEffect(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    spellId = currentSequence.spell_action.spell_id
    spellPacket = tpdp.SpellPacket(spellId)
    if verifyEldritchBlastAction(spellPacket.spell_enum):
        spellDc = spellPacket.dc
        saveType = D20_Save_Reflex
        saveDescriptor = D20STD_F_SPELL_DESCRIPTOR_COLD
        spellTarget = evt_obj.attack_packet.target
        if spellTarget.saving_throw_spell(spellDc, saveType, saveDescriptor, spellPacket.caster, spellId): #success
            spellTarget.float_mesfile_line("mes\\spell.mes", 30001)
            game.particles("Fizzle", spellTarget)
        else:
            spellTarget.float_mesfile_line("mes\\spell.mes", 30002)
            duration = 100 #10 mins
            spellTarget.condition_add_with_args("Hellrime Blast Effect", spellId, duration, 0)
    return 0

hellrimeBlast = EldritchBlastEssenceModifier("sp-Hellrime Blast") #spellEnum, empty
hellrimeBlast.ModifyDamageType(D20DT_COLD)
hellrimeBlast.AddQuerySecondaryTrue()

### Secondary Burn Effect ###

hellrimeEffect = SpellPythonModifier("Hellrime Blast Effect") #spellId, duration, empty
hellrimeEffect.AddAbilityBonus(-4, bonus_type_hellrime_blast, stat_dexterity)
