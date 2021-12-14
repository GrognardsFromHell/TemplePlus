from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions
import feat_utils

# Draconic Arcane Grace: Races of the Dragon, p. 102

print "Registering Draconic Arcane Grace"

draconicArcaneGraceEnum = 8411


def arcaneGraceRadial(attachee, args, evt_obj):
    spellNodeId = []
    spontaneousSpells = attachee.spells_known
    memorizedSpells = attachee.spells_memorized
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
    #This part needs rework
    #As only arcane spells are valid spells to convert
    for spell in spontaneousSpells:
        if spell.is_naturally_cast() and (spell.spell_level > 0) and attachee.spontaneous_spells_remaining(spell.spell_class, spell.spell_level):
            radialId = tpdp.RadialMenuEntryPythonAction(spell, D20A_PYTHON_ACTION, draconicArcaneGraceEnum, spell.spell_enum, featTag)
            radialId.add_as_child(attachee, spellNodeId[spell.spell_level-1])
    for spell in memorizedSpells:
        if (not spell.is_used_up()) and (spell.spell_level > 0):
            radialId = tpdp.RadialMenuEntryPythonAction(spell, D20A_PYTHON_ACTION, draconicArcaneGraceEnum, spell.spell_enum, featTag)
            radialId.add_as_child(attachee, spellNodeId[spell.spell_level-1])
    return 0

def arcaneGracePerform(attachee, args, evt_obj):
    featName = feat_utils.getFeatName(args)
    featTag = feat_utils.getFeatTag(featName)
    spellPacket = tpdp.SpellPacket(attachee, evt_obj.d20a.spell_data)
    ### workaround ###
    # spellPacket.spell_enum isn't carried over
    print "spellPacket: {}".format(spellPacket)
    print "spellPacket.spell_enum: {}".format(spellPacket.spell_enum)
    print "spellPacket.spell_known_slot_level: {}".format(spellPacket.spell_known_slot_level)
    spellPacket.spell_enum = evt_obj.d20a.data1
    ### workaround end ###
    arcaneGraceSaveBonus = args.get_arg(1)
    convertedSpellLevel = spellPacket.spell_known_slot_level
    game.create_history_freeform("{} activates ~{}~[{}]\n\n".format(attachee.description, featName, featTag))
    particlesId = args.get_arg(3)
    if particlesId:
        game.particles_end(particlesId)
    particlesId = game.particles('sp-Chill Touch', attachee)
    if arcaneGraceSaveBonus < convertedSpellLevel:
        args.set_arg(1, convertedSpellLevel)
    #Debit spell that was given up for Draconic Arcane Grace
    spellPacket.debit_spell()
    args.set_arg(3, particlesId)
    return 0

def arcaneGraceSavingThrowBonus(attachee, args, evt_obj):
    arcaneGraceSaveBonus = args.get_arg(1)
    if arcaneGraceSaveBonus:
        featName = feat_utils.getFeatName(args)
        featTag = feat_utils.getFeatTag(featName)
        bonusType = 0 #ID 0 = untyped (stacking)
        evt_obj.bonus_list.add(arcaneGraceSaveBonus, bonusType, "~{}~[{}]".format(featName, featTag))
        #Reset after use but only if it was not called from the character sheet
        if evt_obj.flags != 0:
            particlesId = args.get_arg(3)
            game.particles_end(particlesId)
            args.set_arg(1, 0)
            args.set_arg(3, 0)
    return 0

def arcaneGraceToolTip(attachee, args, evt_obj):
    arcaneGraceSaveBonus = args.get_arg(1)
    if arcaneGraceSavingThrowBonus:
        featName = feat_utils.getFeatName(args)
        evt_obj.append("{} (+ {})".format(featName, arcaneGraceSavingThrowBonus))
    return 0

def arcaneGraceEffectTooltip(attachee, args, evt_obj):
    arcaneGraceSaveBonus = args.get_arg(1)
    if arcaneGraceSavingThrowBonus:
        evt_obj.append(tpdp.hash("DRACONIC_ARCANE_GRACE"), -2, "(+ {})".format(arcaneGraceSavingThrowBonus))
    return 0

draconicArcaneGraceFeat = PythonModifier("Draconic Arcane Grace", 4) #FeatEnum, arcaneGraceSaveBonus, empty, particlesId
draconicArcaneGraceFeat.MapToFeat("Draconic Arcane Grace", feat_cond_arg2 = 0)
draconicArcaneGraceFeat.AddHook(ET_OnSaveThrowLevel, EK_NONE, arcaneGraceSavingThrowBonus, ())
draconicArcaneGraceFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, arcaneGraceRadial, ())
draconicArcaneGraceFeat.AddHook(ET_OnD20PythonActionPerform, draconicArcaneGraceEnum, arcaneGracePerform, ())
draconicArcaneGraceFeat.AddHook(ET_OnGetTooltip, EK_NONE, arcaneGraceToolTip, ())
draconicArcaneGraceFeat.AddHook(ET_OnGetEffectTooltip, EK_NONE, arcaneGraceEffectTooltip, ())
