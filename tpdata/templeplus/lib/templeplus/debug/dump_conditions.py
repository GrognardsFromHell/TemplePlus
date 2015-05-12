import sys

import debug
import re


DISP_TYPES = [
    "TYPE_0",
    "ConditionAdd",
    "ConditionRemove",
    "ConditionAddPre",
    "ConditionRemove2",
    "TYPE_5",
    "TYPE_6",
    "TurnBasedStatusInit",
    "TYPE_8",
    "TYPE_9",
    "AbilityScoreLevel",
    "TYPE_11",
    "TYPE_12",
    "TYPE_13",
    "TYPE_14",
    "TYPE_15",
    "TYPE_16",
    "TYPE_17",
    "TYPE_18",
    "TYPE_19",
    "TYPE_20",
    "TYPE_21",
    "TYPE_22",
    "TYPE_23",
    "TYPE_24",
    "CurrentHP",
    "MaxHP",
    "TYPE_27",
    "D20Signal",
    "D20Query",
    "SkillLevel",
    "RadialMenuEntry",
    "TYPE_32",
    "TYPE_33",
    "TYPE_34",
    "TYPE_35",
    "TYPE_36",
    "TYPE_37",
    "TYPE_38",
    "TYPE_39",
    "TYPE_40",
    "TYPE_41",
    "TYPE_42",
    "TYPE_43",
    "TYPE_44",
    "TYPE_45",
    "TYPE_46",
    "TYPE_47",
    "TYPE_48",
    "TYPE_49",
    "TYPE_50",
    "TYPE_51",
    "TYPE_52",
    "TYPE_53",
    "TYPE_54",
    "TYPE_55",
    "TYPE_56",
    "TYPE_57",
    "TYPE_58",
    "TYPE_59",
    "TYPE_60",
    "TYPE_61",
    "TYPE_62",
    "TYPE_63",
    "TYPE_64",
    "TYPE_65",
    "TYPE_66",
    "TYPE_67",
    "TYPE_68",
    "TYPE_69",
    "TYPE_70",
    "TYPE_71",
    "TYPE_72"
]

D20_KEYS = {
    0x0 : "DK_NONE",
    1 : "DK_STAT_STRENGTH",
    2 : "DK_STAT_DEXTERITY",
    3 : "DK_STAT_CONSTITUTION",
    4 : "DK_STAT_INTELLIGENCE",
    5 : "DK_STAT_WISDOM",
    6 : "DK_STAT_CHARISMA",
    20 : "DK_SKILL_APPRAISE",
    21 : "DK_SKILL_BLUFF",
    22 : "DK_SKILL_CONCENTRATION",
    59 : "DK_SKILL_RIDE",
    60 : "DK_SKILL_SWIM",
    61 : "DK_SKILL_USE_ROPE",
    147 : "DK_SIG_HP_Changed",
    0x94 : "DK_SIG_HealSkill",
    0x95 : "DK_SIG_Sequence",
    0x96 : "DK_SIG_Pre_Action_Sequence",
    0x97 : "DK_SIG_Action_Recipient",
    0x98 : "DK_SIG_BeginTurn",
    0x99 : "DK_SIG_EndTurn",
    0x9A : "DK_SIG_Dropped_Enemy",
    0x9B : "DK_SIG_Concentration_Broken",
    0x9C : "DK_SIG_Remove_Concentration",
    0x9D : "DK_SIG_BreakFree",
    0x9E : "DK_SIG_Spell_Cast",
    0x9F : "DK_SIG_Spell_End",
    0xA0 : "DK_SIG_Spell_Grapple_Removed",
    0xA1 : "DK_SIG_Killed",
    0xA2 : "DK_SIG_AOOPerformed",
    0xA3 : "DK_SIG_Aid_Another",
    0xA4 : "DK_SIG_TouchAttackAdded",
    0xA5 : "DK_SIG_TouchAttack",
    0xA6 : "DK_SIG_Temporary_Hit_Points_Removed",
    0xA7 : "DK_SIG_Standing_Up",
    0xA8 : "DK_SIG_Bardic_Music_Completed",
    0xA9 : "DK_SIG_Combat_End",
    0xAA : "DK_SIG_Initiative_Update",
    0xAB : "DK_SIG_RadialMenu_Clear_Checkbox_Group",
    0xAC : "DK_SIG_Combat_Critter_Moved",
    0xAD : "DK_SIG_Hide",
    0xAE : "DK_SIG_Show",
    0xAF : "DK_SIG_Feat_Remove_Slippery_Mind",
    0xB0 : "DK_SIG_Broadcast_Action",
    0xB1 : "DK_SIG_Remove_Disease",
    0xB2 : "DK_SIG_Rogue_Skill_Mastery_Init",
    0xB3 : "DK_SIG_Spell_Call_Lightning",
    0xB4 : "DK_SIG_Magical_Item_Deactivate",
    0xB5 : "DK_SIG_Spell_Mirror_Image_Struck",
    0xB6 : "DK_SIG_Spell_Sanctuary_Attempt_Save",
    0xB7 : "DK_SIG_Experience_Awarded",
    0xB8 : "DK_SIG_Pack",
    0xB9 : "DK_SIG_Unpack",
    0xBA : "DK_SIG_Teleport_Prepare",
    0xBB : "DK_SIG_Teleport_Reconnect",
    0xBC : "DK_SIG_Atone_Fallen_Paladin",
    0xBD : "DK_SIG_Summon_Creature",
    0xBE : "DK_SIG_Attack_Made",
    0xBF : "DK_SIG_Golden_Skull_Combine",
    0xC0 : "DK_SIG_Inventory_Update",
    0xC1 : "DK_SIG_Critter_Killed",
    0xC2 : "DK_SIG_SetPowerAttack",
    0xC3 : "DK_SIG_SetExpertise",
    0xC4 : "DK_SIG_SetCastDefensively",
    0xC5 : "DK_SIG_Resurrection",
    0xC6 : "DK_SIG_Dismiss_Spells",
    0xC7 : "DK_SIG_DealNormalDamage",
    0xC8 : "DK_SIG_Update_Encumbrance",
    0xC9 : "DK_SIG_Remove_AI_Controlled",
    0xCA : "DK_SIG_Verify_Obj_Conditions",
    0xCB : "DK_SIG_Web_Burning",
    0xCC : "DK_SIG_Anim_CastConjureEnd",
    0xCD : "DK_SIG_Item_Remove_Enhancement",
    0xCF : "DK_QUE_Helpless",
    0xD0 : "DK_QUE_SneakAttack",
    0xD1 : "DK_QUE_OpponentSneakAttack",
    0xD2 : "DK_QUE_CoupDeGrace",
    0xD3 : "DK_QUE_Mute",
    0xD4 : "DK_QUE_CannotCast",
    0xD5 : "DK_QUE_CannotUseIntSkill",
    0xD6 : "DK_QUE_CannotUseChaSkill",
    0xD7 : "DK_QUE_RapidShot",
    0xD8 : "DK_QUE_Critter_Is_Concentrating",
    0xD9 : "DK_QUE_Critter_Is_On_Consecrate_Ground",
    0xDA : "DK_QUE_Critter_Is_On_Desecrate_Ground",
    0xDB : "DK_QUE_Critter_Is_Held",
    0xDC : "DK_QUE_Critter_Is_Invisible",
    0xDD : "DK_QUE_Critter_Is_Afraid",
    0xDE : "DK_QUE_Critter_Is_Blinded",
    0xDF : "DK_QUE_Critter_Is_Charmed",
    0xE0 : "DK_QUE_Critter_Is_Confused",
    0xE1 : "DK_QUE_Critter_Is_AIControlled",
    0xE2 : "DK_QUE_Critter_Is_Cursed",
    0xE3 : "DK_QUE_Critter_Is_Deafened",
    0xE4 : "DK_QUE_Critter_Is_Diseased",
    0xE5 : "DK_QUE_Critter_Is_Poisoned",
    0xE6 : "DK_QUE_Critter_Is_Stunned",
    0xE7 : "DK_QUE_Critter_Is_Immune_Critical_Hits",
    0xE8 : "DK_QUE_Critter_Is_Immune_Poison",
    0xE9 : "DK_QUE_Critter_Has_Spell_Resistance",
    0xEA : "DK_QUE_Critter_Has_Condition",
    0xEB : "DK_QUE_Critter_Has_Freedom_of_Movement",
    0xEC : "DK_QUE_Critter_Has_Endure_Elements",
    0xED : "DK_QUE_Critter_Has_Protection_From_Elements",
    0xEE : "DK_QUE_Critter_Has_Resist_Elements",
    0xEF : "DK_QUE_Critter_Has_True_Seeing",
    0xF0 : "DK_QUE_Critter_Has_Spell_Active",
    0xF1 : "DK_QUE_Critter_Can_Call_Lightning",
    0xF2 : "DK_QUE_Critter_Can_See_Invisible",
    0xF3 : "DK_QUE_Critter_Can_See_Darkvision",
    0xF4 : "DK_QUE_Critter_Can_See_Ethereal",
    0xF5 : "DK_QUE_Critter_Can_Discern_Lies",
    0xF6 : "DK_QUE_Critter_Can_Detect_Chaos",
    0xF7 : "DK_QUE_Critter_Can_Detect_Evil",
    0xF8 : "DK_QUE_Critter_Can_Detect_Good",
    0xF9 : "DK_QUE_Critter_Can_Detect_Law",
    0xFA : "DK_QUE_Critter_Can_Detect_Magic",
    0xFB : "DK_QUE_Critter_Can_Detect_Undead",
    0xFC : "DK_QUE_Critter_Can_Find_Traps",
    0xFD : "DK_QUE_Critter_Can_Dismiss_Spells",
    0xFE : "DK_QUE_Obj_Is_Blessed",
    0xFF : "DK_QUE_Unconscious",
    0x100 : "DK_QUE_Dying",
    0x101 : "DK_QUE_Dead",
    0x102 : "DK_QUE_AOOPossible",
    0x103 : "DK_QUE_AOOIncurs",
    0x104 : "DK_QUE_HoldingCharge",
    0x105 : "DK_QUE_Has_Temporary_Hit_Points",
    0x106 : "DK_QUE_SpellInterrupted",
    0x107 : "DK_QUE_ActionTriggersAOO",
    0x108 : "DK_QUE_ActionAllowed",
    0x109 : "DK_QUE_Prone",
    0x10A : "DK_QUE_RerollSavingThrow",
    0x10B : "DK_QUE_RerollAttack",
    0x10C : "DK_QUE_RerollCritical",
    0x10D : "DK_QUE_Commanded",
    0x10E : "DK_QUE_Turned",
    0x10F : "DK_QUE_Rebuked",
    0x110 : "DK_QUE_CanBeFlanked",
    0x111 : "DK_QUE_Critter_Is_Grappling",
    0x112 : "DK_QUE_Barbarian_Raged",
    0x113 : "DK_QUE_Barbarian_Fatigued",
    0x114 : "DK_QUE_NewRound_This_Turn",
    0x115 : "DK_QUE_Flatfooted",
    0x116 : "DK_QUE_Masterwork",
    0x117 : "DK_QUE_FailedDecipherToday",
    0x118 : "DK_QUE_Polymorphed",
    0x119 : "DK_QUE_IsActionInvalid_CheckAction",
    0x11A : "DK_QUE_CanBeAffected_PerformAction",
    0x11B : "DK_QUE_CanBeAffected_ActionFrame",
    0x11C : "DK_QUE_AOOWillTake",
    0x11D : "DK_QUE_Weapon_Is_Mighty_Cleaving",
    0x11E : "DK_QUE_Autoend_Turn",
    0x11F : "DK_QUE_ExperienceExempt",
    0x120 : "DK_QUE_FavoredClass",
    0x121 : "DK_QUE_IsFallenPaladin",
    0x122 : "DK_QUE_WieldedTwoHanded",
    0x123 : "DK_QUE_Critter_Is_Immune_Energy_Drain",
    0x124 : "DK_QUE_Critter_Is_Immune_Death_Touch",
    0x125 : "DK_QUE_Failed_Copy_Scroll",
    0x126 : "DK_QUE_Armor_Get_AC_Bonus",
    0x127 : "DK_QUE_Armor_Get_Max_DEX_Bonus",
    0x128 : "DK_QUE_Armor_Get_Max_Speed",
    0x129 : "DK_QUE_FightingDefensively",
    0x12A : "DK_QUE_Elemental_Gem_State",
    0x12B : "DK_QUE_Untripable",
    0x12C : "DK_QUE_Has_Thieves_Tools",
    0x12D : "DK_QUE_Critter_Is_Encumbered_Light",
    0x12E : "DK_QUE_Critter_Is_Encumbered_Medium",
    0x12F : "DK_QUE_Critter_Is_Encumbered_Heavy",
    0x130 : "DK_QUE_Critter_Is_Encumbered_Overburdened",
    0x131 : "DK_QUE_Has_Aura_Of_Courage",
    0x132 : "DK_QUE_BardicInstrument",
    0x133 : "DK_QUE_EnterCombat",
    0x134 : "DK_QUE_AI_Fireball_OK",
    0x135 : "DK_QUE_Critter_Cannot_Loot",
    0x136 : "DK_QUE_Critter_Cannot_Wield_Items",
    0x137 : "DK_QUE_Critter_Is_Spell_An_Ability",
    0x138 : "DK_QUE_Play_Critical_Hit_Anim",
    0x139 : "DK_QUE_Is_BreakFree_Possible",
    0x13A : "DK_QUE_Critter_Has_Mirror_Image",
    0x13B : "DK_QUE_Wearing_Ring_of_Change",
    0x13C : "DK_QUE_Critter_Has_No_Con_Score",
    0x13D : "DK_QUE_Item_Has_Enhancement_Bonus",
    0x13E : "DK_QUE_Item_Has_Keen_Bonus",
    0x13F : "DK_QUE_AI_Has_Spell_Override",
    0x140 : "DK_QUE_Weapon_Get_Keen_Bonus",
}

def get_disp_type(dispType):
    return DISP_TYPES[dispType]

def get_disp_key(dispType, dispKey):
    key = "0x%x" % (dispKey, )
    if type == "D20Signal" or type == "D20Query":
        key = D20_KEYS[dispKey]
    if dispKey == 0:
        return None
    return key

def dump_markdown(conds):
    out = open("conditions.md", "wt")

    for (name, numArgs, hooks) in conds:
        out.write("## %s\n\n" % (name,))
        out.write("Arguments: %d\n\n" % (numArgs,))
        out.write("**Dispatcher Hooks:**\n\n")

        out.write("Dispatcher Type | Key | Data1 | Data2 | Callback\n")
        out.write("--- | --- | --- | --- | ---\n")
        for (callback, data1, data2, dispType, dispKey) in hooks:
            type = get_disp_type(dispType)
            key = get_disp_key(type, dispKey)
            if not key:
                key = "&mdash;"
            out.write("%s | %s | 0x%x | 0x%x | 0x%08x\n" % (type, key, data1, data2, callback))
        out.write("\n\n")

    out.close()

def mangle(t):
    t = t.lower()
    return re.sub("\\W", "_", t)

def run():
    conds = debug.dump_conds()
    # sort by name
    conds = sorted(conds, key=lambda x: x[0])
    dump_markdown(conds)


    # find callbacks that are only used by a single condition
    callbacksToCond = {}
    for (name, numArgs, hooks) in conds:
        for (callback, data1, data2, dispType, dispKey) in hooks:
            if callback not in callbacksToCond:
                callbacksToCond[callback] = set()
            type = get_disp_type(dispType)
            key = get_disp_key(type, dispKey)
            if key:
                callbacksToCond[callback].add("cb_%s_%s_%s" % (mangle(name), type.lower(), key.lower()))
            else:
                callbacksToCond[callback].add("cb_%s_%s" % (mangle(name), type.lower()))

    fh = open("unique_callbacks.txt", "wt")
    fh.write(str(callbacksToCond))
    for (callback, c) in callbacksToCond.iteritems():
        if len(c) == 1:
            fh.write("%x: %s\n" % (callback, next(iter(c))))
    fh.close()

    sys.exit()
