from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions

# Fey Legacy: Complete Mage, p. 43

print "Registering Fey Legacy"

feyLegacyEnum = 8551

def feyLegacyResetCharges(attachee, args, evt_obj):
    args.set_arg(2, 0)
    args.set_arg(3, 0)
    args.set_arg(4, 0)
    return 0

def getSpellName(spellEnum):
    return game.get_spell_mesline(spellEnum)

def getSpellTag(spellName):
    return "TAG_SPELLS_{}".format(spellName.upper().replace(" ", "_"))

def getRadialChild(spellEnum, chargesLeft):
    spellName = getSpellName(spellEnum)
    spellTag = getSpellTag(spellName)
    spellRadialId = tpdp.RadialMenuEntryPythonAction("{} {}/1".format(spellName, chargesLeft), D20A_PYTHON_ACTION, feyLegacyEnum, spellEnum, spellTag)
    spellData = tpdp.D20SpellData(spellEnum)
    spellData.set_spell_class(stat_level_sorcerer)
    spellClass = spellData.spell_class
    if spellEnum == spell_summon_natures_ally_v:
        spellData.set_spell_level(5)
    else:
        spellEntry = tpdp.SpellEntry(spellEnum)
        spellLevel = spellEntry.level_for_spell_class(spellClass)
        spellData.set_spell_level(spellLevel)
    spellRadialId.set_spell_data(spellData)
    return spellRadialId

def feyLegacyRadial(attachee, args, evt_obj):
    # Radial Top
    radialSpellLikeTop = tpdp.RadialMenuEntryParent("Fey Legacy")
    radialSpellLikeTopId = radialSpellLikeTop.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
    # Add spell childs
    # Confusion
    spellEnum = spell_confusion
    usedFlagConfusion = args.get_arg(2)
    chargesLeft = "0" if usedFlagConfusion else "1"
    spellRadialId = getRadialChild(spellEnum, chargesLeft)
    spellRadialId.add_as_child(attachee, radialSpellLikeTopId)
    # Dimension Door
    spellEnum = spell_dimension_door
    usedFlagDD = args.get_arg(3)
    chargesLeft = "1" if not usedFlagDD else "0"
    spellRadialId = getRadialChild(spellEnum, chargesLeft)
    spellRadialId.add_as_child(attachee, radialSpellLikeTopId)
    # Summon Natures Ally V
    spellEnum = spell_summon_natures_ally_v 
    usedFlagSummon = args.get_arg(4)
    chargesLeft = "1" if not usedFlagSummon else "0"
    spellRadialId = getRadialChild(spellEnum, chargesLeft)
    spellRadialId.add_as_child(attachee, radialSpellLikeTopId)
    return 0

def feyLegacyCheck(attachee, args, evt_obj):
    spellEnum = evt_obj.d20a.data1
    if spellEnum == spell_confusion:
        usedFlag = args.get_arg(2)
    elif spellEnum == spell_dimension_door:
        usedFlag = args.get_arg(3)
    elif spellEnum == spell_summon_natures_ally_v:
        usedFlag = args.get_arg(4)
    if usedFlag:
        evt_obj.return_val = AEC_OUT_OF_CHARGES
    return 0

def feyLegacyPerform(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    spellPacket = currentSequence.spell_packet
    newSpellId = tpactions.get_new_spell_id()
    spellPacket.caster_level += attachee.stat_level_get(stat_level)
    tpactions.register_spell_cast(spellPacket, newSpellId)
    currentSequence.spell_packet.spell_id = newSpellId

    if attachee.anim_goal_throw_spell_w_cast_anim(): # note: the animation goal has internal calls to trigger_spell_effect and the action frame
        new_anim_id = attachee.anim_goal_get_new_id()
        evt_obj.d20a.flags |= D20CAF_NEED_ANIM_COMPLETED
        evt_obj.d20a.anim_id = new_anim_id
    return 0

def feyLegacyFrame(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    spellPacket = currentSequence.spell_packet
    spellEnum = spellPacket.spell_enum
    #Set used flag
    if spellEnum == spell_confusion:
        args.set_arg(2, 1)
    elif spellEnum == spell_dimension_door:
        args.set_arg(3, 1)
    elif spellEnum == spell_summon_natures_ally_v:
        args.set_arg(4, 1)
    return 0

feyLegacyFeat = PythonModifier("Fey Legacy Feat", 6) #featEnum, heritage, usedFlagConfusion, usedFlagDD, usedFlagSummon, empty
feyLegacyFeat.MapToFeat("Fey Legacy", feat_cond_arg2 = heritage_fey)
feyLegacyFeat.AddHook(ET_OnConditionAdd, EK_NONE, feyLegacyResetCharges, ())
feyLegacyFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, feyLegacyResetCharges, ())
feyLegacyFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, feyLegacyRadial, ())
feyLegacyFeat.AddHook(ET_OnD20PythonActionCheck, feyLegacyEnum, feyLegacyCheck, ())
feyLegacyFeat.AddHook(ET_OnD20PythonActionPerform, feyLegacyEnum, feyLegacyPerform, ())
feyLegacyFeat.AddHook(ET_OnD20PythonActionFrame, feyLegacyEnum, feyLegacyFrame, ())
