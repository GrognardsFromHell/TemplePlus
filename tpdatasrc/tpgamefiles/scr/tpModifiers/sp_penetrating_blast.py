from toee import *
import tpdp
import tpactions
from warlock import EldritchBlastEssenceModifier, verifyEldritchBlastAction, EldritchBlastSecondaryEffect

print "Registering sp-Penetrating Blast"

def secondaryEffect(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    spellId = currentSequence.spell_action.spell_id
    spellPacket = tpdp.SpellPacket(spellId)
    if verifyEldritchBlastAction(spellPacket.spell_enum):
        spellDc = spellPacket.dc
        saveType = D20_Save_Will
        saveDescriptor = D20STD_F_NONE
        spellTarget = evt_obj.attack_packet.target
        if spellTarget.saving_throw_spell(spellDc, saveType, saveDescriptor, spellPacket.caster, spellId): #success
            spellTarget.float_mesfile_line("mes\\spell.mes", 30001)
            game.particles("Fizzle", spellTarget)
        else:
            spellTarget.float_mesfile_line("mes\\spell.mes", 30002)
            duration = 10 #1 minute
            stanceEnum = args.get_arg(0)
            spellTarget.condition_add_with_args("Penetrating Blast Effect", spellId, duration, stanceEnum, 0)
    return 0

def spellPenBonus(attachee, args, evt_obj):
    bonusValue = 4
    bonusType = bonus_type_untyped #Stacking!
    bonusLabel = "~Penetrating Blast~[TAG_SPELLS_PENETRATING_BLAST]"
    evt_obj.bonus_list.add(bonusValue, bonusType, bonusLabel)
    return 0

penetratingBlast = EldritchBlastEssenceModifier("Penetrating Blast") #spellEnum, particlesId, empty
penetratingBlast.AddHook(ET_OnDealingDamage2, EK_NONE, secondaryEffect, ())
penetratingBlast.AddHook(ET_OnSpellResistanceCheckBonus, EK_NONE, spellPenBonus, ())
penetratingBlast.AddQuerySecondaryTrue()

def srPenalty(attachee, args, evt_obj):
    bonusValue = -5
    bonusType = bonus_type_untyped #Stacking!
    bonusLabel = "~Penetrating Blast~[TAG_SPELLS_PENETRATING_BLAST]"
    evt_obj.bonus_list.add(bonusValue, bonusType, bonusLabel)
    return 0

penetratingEffect = EldritchBlastSecondaryEffect("Penetrating Blast Effect") #spellId, duration, secondaryEffectEnum, empty
penetratingEffect.AddHook(ET_OnGetSpellResistanceMod, EK_NONE, srPenalty, ())
penetratingEffect.AddSpellNoDuplicate()