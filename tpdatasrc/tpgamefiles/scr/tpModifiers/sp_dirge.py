from toee import *
import tpdp
from spell_utils import AoeSpellHandleModifier, AoeSpellEffectModifier
print "Registering sp-Dirge"


dirgeSpell = AoeSpellHandleModifier("sp-Dirge", aoe_event_target_non_friendly) #spellId, duration, bonusValue, spellEventId, spellDc, empty


### Start Dirge Effect ###

def dirgeConditionBeginRound(attachee, args, evt_obj):
    if args.get_arg(1) < 0:
        spellId = args.get_arg(0)
        spellDc = args.get_arg(4)
        spellPacket = tpdp.SpellPacket(spellId)
        spellCaster = spellPacket.caster

        attachee.float_text_line("Dirge", tf_red)
        game.create_history_freeform("{} saves versus ~Dirge~[TAG_SPELLS_Dirge]:\n\n".format(attachee.description))
        if attachee.saving_throw_spell(spellDc, D20_Save_Fortitude, D20STD_F_SPELL_DESCRIPTOR_SONIC, spellPacket.caster, spellId):
            attachee.float_mesfile_line('mes\\spell.mes', 30001)
        else:
            attachee.float_mesfile_line('mes\\spell.mes', 30002)
            attacheePartsysId = game.particles('sp-Poison', attachee)
            attachee.condition_add_with_args('Temp_Ability_Loss', stat_strength, 2)
            attachee.condition_add_with_args('Temp_Ability_Loss', stat_dexterity, 2)
            game.create_history_freeform("Abilities damaged\n\n")
    return 0

dirgeCondition = AoeSpellEffectModifier("Dirge") #spellId, duration, bonusValue, spellEventId, spellDc, empty
dirgeCondition.AddHook(ET_OnBeginRound, EK_NONE, dirgeConditionBeginRound, ())

## End Dirge Effect ###
