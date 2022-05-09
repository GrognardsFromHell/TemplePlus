from toee import *
import tpdp
from templeplus.pymod import BasicPyMod
from spell_utils import SpellPythonModifier

print("Registering sp-Retributive Invisibility")

def addInvisCondition(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    duration = args.get_arg(1)
    fadeValue = 128
    if attachee.condition_add_with_args("Invisible", spellId, duration, fadeValue):
        attachee.float_mesfile_line("mes\\spell.mes", 20017) # ID 20017 = Invisible!
    return 0

def queryCondition(attachee, args, evt_obj):
    queryConditionRef = evt_obj.data1
    spellInvisibilityCond = "sp-Invisibility"
    spellConditionRef = tpdp.get_condition_ref(spellInvisibilityCond)
    if queryConditionRef == spellConditionRef:
        evt_obj.return_val = 1
    return 0

def signalDispelled(attachee, args, evt_obj):
    signalId = evt_obj.data1
    spellId = args.get_arg(0)
    if signalId == spellId:
        game.particles("sp-Retributive Invisibility-dispel", attachee)
        spellId = args.get_arg(0)
        spellPacket = tpdp.SpellPacket(spellId)
        spellDc = spellPacket.dc
        saveType = D20_Save_Fortitude
        saveDescriptor = D20STD_F_SPELL_DESCRIPTOR_SONIC
        spellCaster = spellPacket.caster
        spellDamageDice = dice_new("1d6")
        spellDamageDice.number = 4
        damageType = D20DT_SONIC
        range = 20
        flags = OLC_CRITTERS
        targetList = game.obj_list_range(attachee.location, range, flags)
        for target in targetList:
            if target == attachee: # skip self!
                continue
            # Saving Throw for half damage and no stun
            if target.saving_throw_spell(spellDc, saveType, saveDescriptor, spellCaster, spellId):
                target.float_mesfile_line("mes\\spell.mes", 30001)
                target.spell_damage_with_reduction(spellCaster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, DAMAGE_REDUCTION_HALF, D20A_CAST_SPELL, spellId)
            else:
                target.float_mesfile_line("mes\\spell.mes", 30002)
                target.spell_damage(spellCaster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spellId)
                duration = 1
                initiative = attachee.get_initiative()
                target.condition_add_with_args("Stunned", duration, initiative)
    return 0

retributiveInvisibility = SpellPythonModifier("sp-Retributive Invisibility") # spellId, duration, empty
retributiveInvisibility.AddHook(ET_OnConditionAdd, EK_NONE, addInvisCondition, ())
retributiveInvisibility.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Condition, queryCondition, ())
retributiveInvisibility.AddHook(ET_OnD20PythonSignal, "PS_Spell_Dispelled", signalDispelled, ())
retributiveInvisibility.AddSpellDismiss()
