from toee import *
import tpdp
from spell_utils import SpellBasicPyMod, performDispelCheck

print("Registering sp-Voracious Dispelling")

def triggerDispel(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    isAoe = args.get_arg(2)
    performDispelCheck(attachee, spellId, isAoe)
    args.remove_spell_mod()
    return 0

def dealDamage(attachee, args, evt_obj):
    dispelledSpellId = evt_obj.data1
    signalId = evt_obj.data2
    spellId = args.get_arg(0)
    if signalId == spellId:
        spellPacket = tpdp.SpellPacket(spellId)
        spellCaster = spellPacket.caster
        dispelledSpellPacket = tpdp.SpellPacket(dispelledSpellId)
        spellLevel = dispelledSpellPacket.spell_known_slot_level
        spellDamageDice = dice_new("1d1")
        spellDamageDice.bonus = spellLevel - 1
        damageType = D20DT_MAGIC
        attachee.spell_damage(spellCaster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spellId)
    return 0

voraciousDispelling = SpellBasicPyMod("sp-Voracious Dispelling", 4) # spell_id, duration, isAoe, empty
voraciousDispelling.AddSpellNoDuplicate()
voraciousDispelling.AddHook(ET_OnConditionAdd, EK_NONE, triggerDispel, ())
voraciousDispelling.AddHook(ET_OnD20PythonSignal, "PS_Spell_Dispelled", dealDamage, ())
