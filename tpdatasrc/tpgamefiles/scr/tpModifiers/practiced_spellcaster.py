from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

print "Registering Practiced Spellcaster"


def PracticedSpellcasterLevelModArcane(attachee, args, evt_obj):
    spell_packet = evt_obj.get_spell_packet()
    if spell_packet.is_divine_spell():
        return 0

    bonVal = 4
    cur_caster_lvl = spell_packet.caster_level

    cur_hd = attachee.hit_dice_num
    bonVal = min(4, cur_hd - cur_caster_lvl)

    if bonVal > 0:
        print "Practiced Spellcaster: Adding to caster level " + str(bonVal)
        evt_obj.return_val += bonVal
    return 0

def PracticedSpellcasterLevelModDivine(attachee, args, evt_obj):
    spell_packet = evt_obj.get_spell_packet()
    if not spell_packet.is_divine_spell():
        return 0

    bonVal = 4
    cur_caster_lvl = spell_packet.caster_level

    cur_hd = attachee.hit_dice_num
    bonVal = min(4, cur_hd - cur_caster_lvl)

    if bonVal > 0:
        print "Practiced Spellcaster: Adding to caster level " + str(bonVal)
        evt_obj.return_val += bonVal
    return 0

def OnAddSpellCastingArcane(attachee, args, evt_obj):
    # arg0 holds the arcane class
    if args.get_arg(0) == 0:
        highestArcane = char_class_utils.GetHighestArcaneClass(attachee)
        args.set_arg(0, highestArcane)
    return 0


def OnAddSpellCastingDivine(attachee, args, evt_obj):
    # arg0 holds the arcane class
    if args.get_arg(0) == 0:
        highestDivine = char_class_utils.GetHighestDivineClass(attachee)
        args.set_arg(0, highestDivine)
    return 0

pracSC_Arcane = PythonModifier("Practiced Spellcaster Feat - Arcane", 2) # args are just-in-case placeholders
pracSC_Arcane.MapToFeat("Practiced Spellcaster - Arcane")
pracSC_Arcane.AddHook(ET_OnGetCasterLevelMod, EK_NONE, PracticedSpellcasterLevelModArcane, ())
pracSC_Arcane.AddHook(ET_OnConditionAdd, EK_NONE, OnAddSpellCastingArcane, ())

pracSC_Divine = PythonModifier("Practiced Spellcaster Feat - Divine", 2) # args are just-in-case placeholders
pracSC_Divine.MapToFeat("Practiced Spellcaster - Divine")
pracSC_Divine.AddHook(ET_OnGetCasterLevelMod, EK_NONE, PracticedSpellcasterLevelModDivine, ())
pracSC_Divine.AddHook(ET_OnConditionAdd, EK_NONE, OnAddSpellCastingDivine, ())