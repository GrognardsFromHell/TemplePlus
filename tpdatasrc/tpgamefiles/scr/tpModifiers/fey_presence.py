from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions

# Fey Presence: Complete Mage, p. 43

print "Registering Fey Presence"

feyPresenceEnum = 8550

def feyPresenceResetCharges(attachee, args, evt_obj):
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
    spellRadialId = tpdp.RadialMenuEntryPythonAction("{} {}/1".format(spellName, chargesLeft), D20A_PYTHON_ACTION, feyPresenceEnum, spellEnum, spellTag)
    spellData = tpdp.D20SpellData(spellEnum)
    spellData.set_spell_class(stat_level_sorcerer)
    spellClass = spellData.spell_class
    spellEntry = tpdp.SpellEntry(spellEnum)
    spellLevel = spellEntry.level_for_spell_class(spellClass)
    spellData.set_spell_level(spellLevel)
    spellRadialId.set_spell_data(spellData)
    return spellRadialId

def feyPresenceRadial(attachee, args, evt_obj):
    # Radial Top
    radialSpellLikeTop = tpdp.RadialMenuEntryParent("Fey Presence")
    radialSpellLikeTopId = radialSpellLikeTop.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
    # Add spell childs
    # Charm Monster
    spellEnum = spell_charm_monster
    usedFlagCharm = args.get_arg(2)
    chargesLeft = "0" if usedFlagCharm else "1"
    spellRadialId = getRadialChild(spellEnum, chargesLeft)
    spellRadialId.add_as_child(attachee, radialSpellLikeTopId)
    # Deep Slumber
    spellEnum = spell_deep_slumber
    usedFlagSlumber = args.get_arg(3)
    chargesLeft = "1" if not usedFlagSlumber else "0"
    spellRadialId = getRadialChild(spellEnum, chargesLeft)
    spellRadialId.add_as_child(attachee, radialSpellLikeTopId)
    # Disguise Self
    # spell not implemented afaik
    return 0

def feyPresenceCheck(attachee, args, evt_obj):
    spellEnum = evt_obj.d20a.data1
    if spellEnum == spell_charm_monster:
        usedFlag = args.get_arg(2)
    elif spellEnum == spell_deep_slumber:
        usedFlag = args.get_arg(3)
    elif spellEnum == spell_disguise_self:
        usedFlag = args.get_arg(4)
    if usedFlag:
        evt_obj.return_val = AEC_OUT_OF_CHARGES
    return 0

def feyPresencePerform(attachee, args, evt_obj):
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

def feyPresenceFrame(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    spellPacket = currentSequence.spell_packet
    spellEnum = spellPacket.spell_enum
    #Set used flag
    if spellEnum == spell_charm_monster:
        args.set_arg(2, 1)
    elif spellEnum == spell_deep_slumber:
        args.set_arg(3, 1)
    elif spellEnum == spell_disguise_self:
        args.set_arg(4, 1)
    return 0

feyPresenceFeat = PythonModifier("Fey Presence Feat", 6) #featEnum, heritage, usedFlagCharm, usedFlagSlumber, usedFlagDisguise, empty
feyPresenceFeat.MapToFeat("Fey Presence", feat_cond_arg2 = heritage_fey)
feyPresenceFeat.AddHook(ET_OnConditionAdd, EK_NONE, feyPresenceResetCharges, ())
feyPresenceFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, feyPresenceResetCharges, ())
feyPresenceFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, feyPresenceRadial, ())
feyPresenceFeat.AddHook(ET_OnD20PythonActionCheck, feyPresenceEnum, feyPresenceCheck, ())
feyPresenceFeat.AddHook(ET_OnD20PythonActionPerform, feyPresenceEnum, feyPresencePerform, ())
feyPresenceFeat.AddHook(ET_OnD20PythonActionFrame, feyPresenceEnum, feyPresenceFrame, ())
