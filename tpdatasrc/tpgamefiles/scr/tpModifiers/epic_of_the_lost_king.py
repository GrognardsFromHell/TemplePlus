from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions

print "Registering Epic of the Lost King"

epicOfTheLostKingEnum = 820

def radialEpicOfTheLostKing(attachee, args, evt_obj):
    maxCharges = attachee.d20_query("Max Bardic Music")
    chargesLeft = attachee.d20_query("Current Bardic Music")
    featName = "Epic of the Lost King"
    featTag = "TAG_EPIC_OF_THE_LOST_KING"
    radialTop = tpdp.RadialMenuEntryParent("{} {}/{}".format(featName, chargesLeft, maxCharges))
    radialTopId = radialTop.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)

    radialId = tpdp.RadialMenuEntryPythonAction("Remove Fatigue", D20A_PYTHON_ACTION, epicOfTheLostKingEnum, 1, featTag) #data1 = requiredCharges
    spellData = tpdp.D20SpellData(spell_epic_of_the_lost_king)
    spellData.set_spell_class(stat_level_bard)
    spellLevel = 1 #Using the spell level to differenciate if spell should remove fatigue or exhaustion
    spellData.set_spell_level(spellLevel)
    radialId.set_spell_data(spellData)
    radialId.add_as_child(attachee, radialTopId)

    radialId = tpdp.RadialMenuEntryPythonAction("Remove Exhaustion", D20A_PYTHON_ACTION, epicOfTheLostKingEnum, 3, featTag) #data1 = requiredCharges
    spellData = tpdp.D20SpellData(spell_epic_of_the_lost_king)
    spellData.set_spell_class(stat_level_bard)
    spellLevel = 3 #Using the spell level to differenciate if spell should remove fatigue or exhaustion
    spellData.set_spell_level(spellLevel)
    radialId.set_spell_data(spellData)
    radialId.add_as_child(attachee, radialTopId)
    return 0

def checkEpicOfTheLostKing(attachee, args, evt_obj):
    chargesLeft = attachee.d20_query("Current Bardic Music")
    requiredCharges = evt_obj.d20a.data1
    if chargesLeft < requiredCharges:
        evt_obj.return_val = AEC_OUT_OF_CHARGES
    return 0

def performEpicOfTheLostKing(attachee, args, evt_obj):
    featName = "Epic of the Lost King"
    featTag = "TAG_EPIC_OF_THE_LOST_KING"
    game.create_history_freeform("{} activates ~{}~[{}]\n\n".format(attachee.description, featName, featTag))

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

def frameEpicOfTheLostKing(attachee, args, evt_obj):
    requiredCharges = evt_obj.d20a.data1
    #The Bardic Deduct Signal doesn't have an option to deduct more than one charge
    #So I sent the signal 3 times, if it should remove exhaustion
    for i in range(0, requiredCharges):
        attachee.d20_send_signal("Deduct Bardic Music Charge")
    return 0

epicOfTheLostKingFeat = PythonModifier("Epic of the Lost King Feat", 2) #featEnum, empty
epicOfTheLostKingFeat.MapToFeat("Epic of the Lost King", feat_cond_arg2 = 0)
epicOfTheLostKingFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, radialEpicOfTheLostKing, ())
epicOfTheLostKingFeat.AddHook(ET_OnD20PythonActionCheck, epicOfTheLostKingEnum, checkEpicOfTheLostKing, ())
epicOfTheLostKingFeat.AddHook(ET_OnD20PythonActionPerform, epicOfTheLostKingEnum, performEpicOfTheLostKing, ())
epicOfTheLostKingFeat.AddHook(ET_OnD20PythonActionFrame, epicOfTheLostKingEnum, frameEpicOfTheLostKing, ())
