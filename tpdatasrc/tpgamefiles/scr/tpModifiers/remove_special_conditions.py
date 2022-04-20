from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from spell_utils import getSpellEntry

#This file contains all helper conditions to remove special conditions
#like fatigue or nauseated that are not already handled by a vanilla spell

### WIP ###

def removeSelfHook(attachee, args, evt_obj):
    #particlesString = args.get_cond_name()
    #game.particles(particlesString, attachee)
    args.remove_spell()
    args.remove_spell_mod()
    return 0

class RemoveConditionModifier(PythonModifier):
    # RemoveConditionModifier have at least 3 arguments:
    # 0: spell_id, 1: duration, 2: empty
    def __init__(self, name, removeSelf = True, args = 3, preventDuplicate = False):
        PythonModifier.__init__(self, name, args, preventDuplicate)
        self.AddSpellTeleportPrepareStandard()
        self.AddSpellTeleportReconnectStandard()
        self.AddSpellCountdownStandardHook()
        if removeSelf:
            self.AddHook(ET_OnConditionAdd, EK_NONE, removeSelfHook, ())
    def AddDispelCheck(self):
        self.AddSpellDispelCheckStandard()


removeDazzled = RemoveConditionModifier("sp-Remove Dazzled")

removeExhaustion = RemoveConditionModifier("sp-Remove Exhaustion")

removeFatigue = RemoveConditionModifier("sp-Remove Fatigue")

removeNauseated = RemoveConditionModifier("sp-Remove Nauseated")

removeShaken = RemoveConditionModifier("sp-Remove Shaken")

removeSickened = RemoveConditionModifier("sp-Remove Sickened")


def blockDarknessEffects(attachee, args, evt_obj):
    if (evt_obj.is_modifier("sp-Assasssins's Darkness") # TDB!
    or evt_obj.is_modifier("sp-Bleakness") # TBD!
    or evt_obj.is_modifier("sp-Darkness")
    or evt_obj.is_modifier("Darkness")
    or evt_obj.is_modifier("sp-Deeper Darkness")
    or evt_obj.is_modifier("Deeper Darkness")
    or evt_obj.is_modifier("sp-Enervating Shadow")
    or evt_obj.is_modifier("sp-Ravenous Darkness") # TBD!
    or evt_obj.is_modifier("sp-Veil of Shadow")):
        newEffSpellId = evt_obj.arg1
        newEffSpellPacket = tpdp.SpellPacket(newEffSpellId)
        newEffSpellLevel = newEffSpellPacket.spell_known_slot_level
        spellId = args.get_arg(0)
        spellPacket = tpdp.SpellPacket(spellId)
        spellLevel = spellPacket.spell_known_slot_level
        if newEffSpellLevel < spellLevel:
            evt_obj.return_val = 0
    return 0

removeDarkness = RemoveConditionModifier("sp-Dispel Darkness", False)
removeDarkness.AddHook(ET_OnConditionAddPre, EK_NONE, blockDarknessEffects, ())


def blockLightEffects(attachee, args, evt_obj):
    if (evt_obj.is_modifier("sp-Blistering Radiance") # TBD!
    or evt_obj.is_modifier("sp-Daylight")
    or evt_obj.is_modifier("Daylight")
    or evt_obj.is_modifier("sp-Radiance")
    or evt_obj.is_modifier("Radiance")
    or evt_obj.is_modifier("sp-Rejuvenating Light")): # TBD!
        newEffSpellId = evt_obj.arg1
        newEffSpellPacket = tpdp.SpellPacket(newEffSpellId)
        newEffSpellLevel = newEffSpellPacket.spell_known_slot_level
        spellId = args.get_arg(0)
        spellPacket = tpdp.SpellPacket(spellId)
        spellLevel = spellPacket.spell_known_slot_level
        if newEffSpellLevel < spellLevel:
            evt_obj.return_val = 0
    return 0

removeLight = RemoveConditionModifier("sp-Dispel Light", False)
removeLight.AddHook(ET_OnConditionAddPre, EK_NONE, blockLightEffects, ())
