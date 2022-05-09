from toee import *
import tpdp
from spell_utils import SpellBasicPyMod, performDispelCheck

print("Registering sp-Devour Magic")

def triggerDispel(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    isAoe = False
    performDispelCheck(attachee, spellId, isAoe)
    args.remove_spell_mod()
    return 0

def signalSpellDispelled(attachee, args, evt_obj):
    dispelledSpellId = evt_obj.data1
    signalId = evt_obj.data2
    spellId = args.get_arg(0)
    if signalId == spellId:
        storedTempHp = args.get_arg(2)
        spellPacket = tpdp.SpellPacket(spellId)
        spellCaster = spellPacket.caster
        if not storedTempHp:
            particlesId = game.particles("sp-Devour Magic-tempHp", spellCaster)
            spellPacket.add_target(spellCaster, particlesId)
        dispelledSpellPacket = tpdp.SpellPacket(dispelledSpellId)
        dispelledSpellLevel = dispelledSpellPacket.spell_known_slot_level
        devouredTempHp = dispelledSpellLevel * 5
        if devouredTempHp >= storedTempHp:
            args.set_arg(2, devouredTempHp)
            duration = 10
            spellCaster.condition_add_with_args("sp-Devour Magic TempHp", spellId, duration, devouredTempHp, 0)
    return 0

def checkRemoveSpell(attachee, args, evt_obj):
    storedTempHp = args.get_arg(2)
    if not storedTempHp:
        args.remove_spell()
    return 0

devourMagic = SpellBasicPyMod("sp-Devour Magic", 4) # spell_id, duration, storedTempHp, empty
devourMagic.AddSpellNoDuplicate()
devourMagic.AddHook(ET_OnConditionAdd, EK_NONE, triggerDispel, ())
devourMagic.AddHook(ET_OnD20PythonSignal, "PS_Spell_Dispelled", signalSpellDispelled, ())
devourMagic.AddHook(ET_OnConditionRemove, EK_NONE, checkRemoveSpell, ())

devourMagicTempHp = SpellBasicPyMod("sp-Devour Magic TempHp", 4) # spell_id, duration, storedTempHp, empty
devourMagicTempHp.AddSpellTooltips()
devourMagicTempHp.AddSpellCountdown()
devourMagicTempHp.AddTempHp(passed_by_spell)
devourMagicTempHp.AddSpellNoDuplicate()
