from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions

# Arcane Strike: Complete Warrior, p. 96

print "Registering Arcane Strike"

arcaneStrikeEnum = 8411

def getFeatName():
    return "Arcane Strike"

def getFeatTag():
    return "TAG_ARCANE_STRIKE"


def arcaneStrikeRadial(attachee, args, evt_obj):
    spellNodeId = []
    knownSpells = attachee.spells_known
    memorizedSpells = attachee.spells_memorized
    featName = getFeatName()
    featTag = getFeatTag()

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
    for spell in knownSpells:
        if (spell.is_naturally_cast()
        and spell.spell_level > 0
        and attachee.spontaneous_spells_remaining(spell.spell_class, spell.spell_level)):
            radialId = tpdp.RadialMenuEntryPythonAction(spell, D20A_PYTHON_ACTION, arcaneStrikeEnum, spell.spell_enum, featTag)
            radialId.add_as_child(attachee, spellNodeId[spell.spell_level-1])
    for spell in memorizedSpells:
        if not spell.is_used_up() and spell.spell_level > 0:
            radialId = tpdp.RadialMenuEntryPythonAction(spell, D20A_PYTHON_ACTION, arcaneStrikeEnum, spell.spell_enum, featTag)
            radialId.add_as_child(attachee, spellNodeId[spell.spell_level-1])
    return 0

def arcaneStrikePerform(attachee, args, evt_obj):
    featName = getFeatName()
    featTag = getFeatTag()
    spellPacket = tpdp.SpellPacket(attachee, evt_obj.d20a.spell_data)
    #currentSequence = tpactions.get_cur_seq()
    #spellPacket = currentSequence.spell_packet
    ### workaround ###
    # spellPacket.spell_enum isn't carried over
    print "spellPacket: {}".format(spellPacket)
    print "spellPacket.spell_enum: {}".format(spellPacket.spell_enum)
    print "spellPacket.spell_known_slot_level: {}".format(spellPacket.spell_known_slot_level)
    spellPacket.spell_enum = evt_obj.d20a.data1
    ### workaround end ###

    #Debit spell that was given up for Draconic Arcane Grace
    spellPacket.debit_spell()

    #Add Effect Condition; Using a Condition instead of adding hooks for every
    #ToHit and Damage steps of attachee
    #Could als be done in one Modifier by using activation Flags for both hooks
    #And reset the activation flag in OnBeginRound
    #But I am unsure if it's really better to hook every onBeginRound
    particlesId = game.particles("ft-Arcane Strike", attachee)
    duration = 0 #will be removed at beginning of next round
    spellLevel = spellPacket.spell_known_slot_level
    attachee.condition_add_with_args("{} Effect".format(featName), duration, spellLevel, particlesId, 0)
    return 0

arcaneStrikeFeat = PythonModifier(getFeatName(), 2) #FeatEnum, empty
arcaneStrikeFeat.MapToFeat(getFeatName(), feat_cond_arg2 = 0)
arcaneStrikeFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, arcaneStrikeRadial, ())
arcaneStrikeFeat.AddHook(ET_OnD20PythonActionPerform, arcaneStrikeEnum, arcaneStrikePerform, ())

def arcaneStrikeTickDown(attachee, args, evt_obj):
    duration = args.get_arg(0)
    duration -= evt_obj.data1
    args.set_arg(0, duration)
    if args.get_arg(0) < 0:
        particlesId = args.get_arg(2)
        game.particles_end(particlesId)
        args.condition_remove()
    return 0

def arcaneStrikeToHitBonus(attachee, args, evt_obj):
    featName = getFeatName()
    featTag = getFeatTag()
    spellLevel = args.get_arg(1)
    bonusValue = spellLevel
    bonusType = 0 # ID 0 = Untyped (stacking)
    evt_obj.bonus_list.add(bonusValue ,bonusType ,"~{}~[{}]".format(featName, featTag))
    return 0

def arcaneStrikeBonusDamage(attachee, args, evt_obj):
    flags = evt_obj.attack_packet.get_flags()
    if not flags & D20CAF_SECONDARY_WEAPON:
        spellLevel = args.get_arg(1)
        damageDice = dice_new('1d4')
        damageDice.number = spellLevel
        damageType = D20DT_UNSPECIFIED #Damage Type will be determinated by original attack
        damageMesId = 4006 #ID 4006 NEW! added in damage_ext.mes
        evt_obj.damage_packet.add_dice(damageDice, damageType, damageMesId)
        #On Hit particles ToDo!
    return 0

def arcaneStrikeToolTip(attachee, args, evt_obj):
    featName = getFeatName()
    evt_obj.append(featName)
    return 0

def arcaneStrikeEffectTooltip(attachee, args, evt_obj):
    featName = getFeatName()
    featKey = featName.upper().replace(" ", "_")
    spellLevel = args.get_arg(1)
    evt_obj.append(featKey, -2, " (Sacrificed Spell Level: {})".format(spellLevel))
    return 0

arcaneStrikeEffect = PythonModifier("{} Effect".format(getFeatName()), 4) #duration, spellLevel, particlesId, empty
arcaneStrikeEffect.AddHook(ET_OnBeginRound, EK_NONE, arcaneStrikeTickDown, ())
arcaneStrikeEffect.AddHook(ET_OnToHitBonus2, EK_NONE, arcaneStrikeToHitBonus, ())
arcaneStrikeEffect.AddHook(ET_OnDealingDamage, EK_NONE, arcaneStrikeBonusDamage, ())
arcaneStrikeEffect.AddHook(ET_OnGetTooltip, EK_NONE, arcaneStrikeToolTip, ())
arcaneStrikeEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, arcaneStrikeEffectTooltip, ())
