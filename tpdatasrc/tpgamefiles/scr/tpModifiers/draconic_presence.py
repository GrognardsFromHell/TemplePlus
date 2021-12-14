from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import feat_utils

# Draconic Presence: Complete Arcane, p. 78
# Draconic Presence: Races of the Dragon, p. 104

print "Registering Draconic Presence"

def verifyTarget(attachee, target):
    if target == attachee:
        return False
    elif target.is_friendly(attachee):
        return False
    elif target.hit_dice_num >= attachee.hit_dice_num:
        return False
    elif target.stat_level_get(stat_intelligence) < 3:
        return False
    elif target.is_category_type(mc_type_dragon):
        return False
    elif target.d20_query("PQ_Is_Shaken"):
        return False
    elif target.d20_query(Q_Critter_Is_Afraid):
        return False
    return True

def applyDraconicPresence(attachee, args, evt_obj):
    spellPacket = evt_obj.get_spell_packet()
    if spellPacket.is_divine_spell() or spellPacket.caster != attachee:
        return 0

    effectRange = 10
    effectRadius = 360
    crittersInRange = game.obj_list_cone(attachee, OLC_CRITTERS, effectRange, 0, effectRadius)
    targetList = []
    for target in crittersInRange:
        if verifyTarget(attachee, target):
            targetList.append(target)

    if not targetList:
        return 0

    for target in targetList:
        spellLevel = spellPacket.spell_known_slot_level #not tested with metamagic hightend spells
        featName = feat_utils.getFeatName(args)
        featTag = feat_utils.getFeatTag(featName)
        game.create_history_freeform("{} saves versus ~{}~[{}]\n\n".format(target.description, featName, featTag))
        charismaModifier = feat_utils.getAbilityModifier(attachee, stat_charisma)
        shakenDc = 10 + spellLevel + charismaModifier
        if target.saving_throw(shakenDc, D20_Save_Will, D20STD_F_SPELL_DESCRIPTOR_FEAR, attachee, D20A_NONE):
            target.float_mesfile_line('mes\\spell.mes', 30001)
            duration = 14400 # 14400 = 1 day
            target.condition_add_with_args("Draconic Presence Immunity", duration, 0)
        else:
            target.float_mesfile_line('mes\\spell.mes', 30002)
            duration = spellLevel
            target.condition_add_with_args("Shaken", duration, 0, 0)
        if not game.combat_is_active():
            target.attack(attachee)
    return 0

draconicPresenceFeat = PythonModifier("Draconic Presence", 0) #FeatEnum, empty
draconicPresenceFeat.MapToFeat("Draconic Presence", feat_cond_arg2 = 0)
draconicPresenceFeat.AddHook(ET_OnGetCasterLevelMod, EK_NONE, applyDraconicPresence, ())

### Draconic Presence Immunity Effect

def applyImmunity(attachee, args, evt_obj):
    if evt_obj.is_modifier("Draconic Presence"):
        attachee.float_text_line("Cannot be affected by Draconic Presence", tf_red)
        evt_obj.return_val = 0
    return 0

def durationTickdown(attachee, args, evt_obj):
    args.set_arg(0, args.get_arg(0)-evt_obj.data1) # Ticking down duration
    if args.get_arg(0) < 0:
        args.condition_remove()
    return 0

def immunityTooltip(attachee, args, evt_obj):
    evt_obj.append("Draconic Presence Immunity")
    return 0

draconicPresenceImmunity = PythonModifier("Draconic Presence Immunity", 2) #duration, empty
draconicPresenceImmunity.AddHook(ET_OnConditionAddPre, EK_NONE, applyImmunity, ())
draconicPresenceImmunity.AddHook(ET_OnBeginRound, EK_NONE, durationTickdown, ())
draconicPresenceImmunity.AddHook(ET_OnGetTooltip, EK_NONE, immunityTooltip, ())
