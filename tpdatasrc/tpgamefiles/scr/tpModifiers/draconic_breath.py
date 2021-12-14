from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions
from heritage_feat_utils import getDraconicHeritageElement
import feat_utils

###################################################

print "Registering Draconic Breath"

# Draconic Breath: Complete Arcane p. 77

draconicBreathEnum = 8500 #Using an enum of the Dragonheart Mage number space

###################################################

def draconicBreathRadial(attachee, args, evt_obj):
    spellNodeId = []
    spontaneousSpells = attachee.spells_known
    #memorizedSpells = attachee.spells_memorized
    heritage = attachee.d20_query("PQ_Selected_Draconic_Heritage")
    heritageElement = getDraconicHeritageElement(heritage)
    if heritageElement == D20DT_COLD or heritageElement == D20DT_FIRE:
        breathEnum = spell_draconic_breath_cone
    elif heritageElement == D20DT_ACID or heritageElement == D20DT_ELECTRICITY:
        breathEnum = spell_draconic_breath_line
    featName = feat_utils.getFeatName(args)
    featTag = feat_utils.getFeatTag(featName)

    #Create Radial Top
    radialTop = tpdp.RadialMenuEntryParent(featName)
    radialTopId = radialTop.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)

    #Create Spell Level Nodes
    for node in range(1,10):
        spellNode = tpdp.RadialMenuEntryParent("{}".format(node))
        spellNodeId.append(spellNode.add_as_child(attachee, radialTopId))

    #Add Radial Childs
    #At the moment this is limited to sorcerer spells only
    #Due to handling debit spell
    for spell in spontaneousSpells:
        if (spell.is_naturally_cast()
        and spell.spell_level > 0
        and spell.spell_class == 144
        and attachee.spontaneous_spells_remaining(spell.spell_class, spell.spell_level)):
            radialId = tpdp.RadialMenuEntryPythonAction(spell, D20A_PYTHON_ACTION, draconicBreathEnum, 0, featTag)
            spellData = tpdp.D20SpellData(breathEnum)
            spellData.set_spell_class(stat_level_sorcerer)
            spellData.set_spell_level(spell.spell_level)
            radialId.set_spell_data(spellData)
            radialId.add_as_child(attachee, spellNodeId[spell.spell_level-1])
    #for spell in memorizedSpells:
    #    if (not spell.is_used_up()) and (spell.spell_level > 0):
    #        radialId = tpdp.RadialMenuEntryPythonAction(spell, D20A_PYTHON_ACTION, draconicBreathEnum, breathEnum, featTag)
    #        radialId.add_as_child(attachee, spellNodeId[spell.spell_level-1])
    return 0

def draconicBreathPerform(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    spellPacket = currentSequence.spell_packet
    newSpellId = tpactions.get_new_spell_id()
    tpactions.register_spell_cast(spellPacket, newSpellId)
    currentSequence.spell_packet.spell_id = newSpellId
    spellPacket.debit_spell()

    if attachee.anim_goal_throw_spell_w_cast_anim(): # note: the animation goal has internal calls to trigger_spell_effect and the action frame
        new_anim_id = attachee.anim_goal_get_new_id()
        evt_obj.d20a.flags |= D20CAF_NEED_ANIM_COMPLETED
        evt_obj.d20a.anim_id = new_anim_id
    return 0

def draconicBreathFrame(attachee, args, evt_obj):
    #Create History
    featName = feat_utils.getFeatName(args)
    featTag = feat_utils.getFeatTag(featName)
    genderString = "his" if attachee.stat_level_get(stat_gender) == 1 else "her"
    game.create_history_freeform("{} uses {} ~{}~[{}] Weapon\n\n".format(attachee.description, genderString, featName, featTag))
    return 0

draconicBreath = PythonModifier("Draconic Breath", 2) #FeatEnum, empty
draconicBreath.MapToFeat("Draconic Breath", feat_cond_arg2 = 0)
draconicBreath.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, draconicBreathRadial, ())
draconicBreath.AddHook(ET_OnD20PythonActionPerform, draconicBreathEnum, draconicBreathPerform, ())
draconicBreath.AddHook(ET_OnD20PythonActionFrame, draconicBreathEnum, draconicBreathFrame, ())
