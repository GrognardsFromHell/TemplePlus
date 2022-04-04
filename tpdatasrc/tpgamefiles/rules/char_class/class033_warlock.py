from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName(): # used by API
    return "Warlock"
    
# def GetSpellCasterConditionName():
    # return "Wizard Spellcasting"

def GetCategory():
    return "Complete Arcane"

def GetClassDefinitionFlags():
    return CDF_BaseClass

def GetClassHelpTopic():
    return "TAG_WARLOCKS"

classEnum = stat_level_warlock

###################################################

class_feats = {
1: (feat_armor_proficiency_light, feat_simple_weapon_proficiency, "Warlock Eldritch Blast", "Warlock Spell Failure", "Warlock Invocations",),
2: ("Warlock Detect Magic",),
3: ("Warlock Damage Reduction",),
8: ("Warlock Fiendish Resilience",),
}

class_skills = (skill_bluff, skill_concentration, skill_craft, skill_disguise, skill_intimidate, skill_jump, skill_knowledge_arcana, skill_knowledge_religion,
skill_profession, skill_sense_motive, skill_spellcraft, skill_use_magic_device) #skill_knowledge_planes is not in the game


#Spell Level 1+2 = Least Invocations
#Spell Level 3+4 = Lesser Invocations
#Spell Level 5+6 = Greater Invocations
#Spell Level 7+ = Dark Invocations

# Moved spells (original spell level):
# spell_bewitching_blast (4)
# spell_hindering_blast (4)
# spell_walk_unseen (2); no save anyways
# spell_cold_comfort (2); no save anyways

spell_list = {
    1: (spell_eldritch_blast, spell_hideous_blow, spell_breath_of_the_night, spell_miasmic_cloud,),
    2: (spell_all_seeing_eyes, spell_beguiling_influence, spell_dark_ones_own_luck, spell_eldritch_spear, spell_frightful_blast,
    spell_entropic_warding, spell_leaps_and_bounds, spell_see_the_unseen, spell_sickening_blast, spell_soulreaving_aura, spell_spiderwalk,),
    3: (spell_brimstone_blast, spell_cold_comfort, spell_fell_flight, spell_walk_unseen, spell_witchwood_step,),
    4: (spell_beshadowed_blast, spell_curse_of_despair, spell_dread_seizure, spell_eldritch_chain, spell_flee_the_scene, spell_hellrime_blast,
    spell_ignore_the_pyre, spell_the_dead_walk, spell_voracious_dispelling, spell_warlock_charm,),
    5: (spell_bewitching_blast, spell_eldritch_cone, spell_eldritch_line, spell_enervating_shadow, spell_hindering_blast,),
    6: (spell_noxious_blast, spell_penetrating_blast, spell_repelling_blast, spell_vitriolic_blast,),
    7: (spell_binding_blast,),
    8: (spell_eldritch_doom, spell_utterdark_blast,)
    #9: ()
    }

bonus_feats =["Warlock Energy Resistance"]

spells_per_day = {
1:  (-1, 1, 1),
2:  (-1, 1, 1),
3:  (-1, 1, 1),
4:  (-1, 1, 1),
5:  (-1, 1, 1),
6:  (-1, 1, 1, 1, 1),
7:  (-1, 1, 1, 1, 1),
8:  (-1, 1, 1, 1, 1),
9:  (-1, 1, 1, 1, 1),
10: (-1, 1, 1, 1, 1),
11: (-1, 1, 1, 1, 1, 1, 1),
12: (-1, 1, 1, 1, 1, 1, 1),
13: (-1, 1, 1, 1, 1, 1, 1),
14: (-1, 1, 1, 1, 1, 1, 1),
15: (-1, 1, 1, 1, 1, 1, 1),
16: (-1, 1, 1, 1, 1, 1, 1, 1, 1, 1),
17: (-1, 1, 1, 1, 1, 1, 1, 1, 1, 1),
18: (-1, 1, 1, 1, 1, 1, 1, 1, 1, 1),
19: (-1, 1, 1, 1, 1, 1, 1, 1, 1, 1),
20: (-1, 1, 1, 1, 1, 1, 1, 1, 1, 1)
#lvl  0  1  2  3  4  5  6  7  8  9
}

def IsEnabled():
    return 1

def GetHitDieType():
    return 6
    
def GetSkillPtsPerLevel():
    return 2
    
def GetBabProgression():
    return base_attack_bonus_type_semi_martial
    
def IsFortSaveFavored():
    return 0
    
def IsRefSaveFavored():
    return 0
    
def IsWillSaveFavored():
    return 1

# Spell casting
def GetSpellListType():
    return spell_list_type_special

def GetSpellList():
    return spell_list

def GetSpellSourceType():
    return spell_source_type_arcane

def GetSpellReadyingType():
    return spell_readying_at_will

def GetSpellsPerDay():
    return spells_per_day

def GetCasterLevels():
    return range(1, 21)

def GetSpellDeterminingStat():
    return stat_charisma

def IsClassSkill(skillEnum):
    return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
    return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
    return class_feats


def IsAlignmentCompatible(alignment):
    if (alignment & ALIGNMENT_EVIL):
        return 1
    elif (alignment & ALIGNMENT_CHAOTIC):
        return 1
    return 0

def ObjMeetsPrereqs(obj):
    return 1

def GetDeityClass():
    return stat_level_wizard

## Levelup callbacks

def IsSelectingFeatsOnLevelup(obj):
    newLevel = obj.stat_level_get(classEnum) + 1
    if newLevel == 10:
        return 1
    ###### Workaround for second Energy Resistance Feat ######
    #Warlock would gain 2 energy resistances at level 10
    #But I can only grant one bonus feat
    #This should be handled as class feature
    #Using this Workaround around until I can use class features
    elif newLevel == 11:
        return 1
    ##### Workaround end ####ä
    return 0

def LevelupGetBonusFeats(obj):
    bonFeatInfo = []
    for ft in bonus_feats:
        bonFeatInfo.append(char_editor.FeatInfo(ft))
    char_editor.set_bonus_feats(bonFeatInfo)
    return 0

def IsSelectingSpellsOnLevelup(obj):
    newLevel = obj.stat_level_get(classEnum) + 1
    newInvocation = [1, 2, 4, 6, 8, 10, 11, 13, 15, 16, 18, 20]
    if newLevel in newInvocation:
        return 1
    return 0

def getMaxSpellLevel(newLevel):
    if newLevel < 6:
        return 2 #Least Invocations
    elif newLevel < 11:
        return 4 #Lesser Invocations
    elif newLevel < 16:
        return 6 #Greater Invocations
    return 9 #Dark Invocations

def addEldritchBlast():
    eldritchBlastEnum = []
    eldritchBlast = char_editor.KnownSpellInfo(spell_eldritch_blast, 3, classEnum)
    eldritchBlastEnum.append(eldritchBlast)
    char_editor.append_spell_enums(eldritchBlastEnum)
    return 0

def replaceInvocation(obj, maxReplaceableSpellLvl):
    #Limiting this for strict level up atm, this was also done in FvS, I assume this has reasons :)
    #At the moment this is limited to replace only Invocations of the same spell level
    #But you should be able to replace any lower level invocation with a new one
    #Example: At level 11 I should be able to swap a least for a lesser invocation
    #This function would also be needed for the ToB Maneuvers.
    if char_editor.get_class_code() == classEnum:
        knownSpells = char_editor.get_known_class_spells(obj, classEnum)
        for label in range(1, maxReplaceableSpellLvl + 1):
            knownSpells.append(char_editor.KnownSpellInfo(spell_label_level_0 + label, 0, classEnum))
        for idx in range(0, len(knownSpells)):
            spellEnum = knownSpells[idx].spell_enum
            if spell_vacant <= spellEnum <= spell_label_level_9:
                continue
            if spell_new_slot_lvl_0 <= spellEnum <= spell_new_slot_lvl_9:
                continue
            if char_editor.get_spell_level(spellEnum, classEnum) <= maxReplaceableSpellLvl:
                knownSpells[idx].spell_status = 1
        knownSpells.sort()
        char_editor.append_spell_enums(knownSpells)
    return 0

def InitSpellSelection(obj, classLvlNew = -1, classLvlIncrement = 1):
    newLevel = char_editor.stat_level_get(classEnum)
    maxSpellLvl = getMaxSpellLevel(newLevel)
    #Get availible spells
    availibleSpells = char_editor.get_learnable_spells(obj, classEnum, maxSpellLvl)
    #Add spell level labels
    for label in range(1, maxSpellLvl + 1):
        availibleSpells.append(char_editor.KnownSpellInfo(spell_label_level_0 + label, 0, classEnum))
    #Fill spell list
    availibleSpells.sort()
    char_editor.append_available_spells(availibleSpells)
    #Add Eldritch Blast as a known spell if first level of Warlock
    #I'll add it here and not in finalize to avoid that Eldritch Blast
    #Is accidently taken, as it is an automatically known spell.
    if newLevel == 1:
        addEldritchBlast()
    #Add Spell slot
    vacantSlotEnum = []
    vacant_slot = char_editor.KnownSpellInfo(spell_vacant, 3, classEnum) # sets it to spell level -1
    vacantSlotEnum.append(vacant_slot)
    char_editor.append_spell_enums(vacantSlotEnum)
    #Handling Invocation Replacements
    #Replace a known Invocation when you learn a new type of Invocation(Lesser, Greater, Dark)
    if newLevel in [6, 11, 16]:
        maxReplaceableSpellLvl = maxSpellLvl - 2
        replaceInvocation(obj, maxReplaceableSpellLvl)
    return 0

def LevelupCheckSpells(obj):
    spell_enums = char_editor.get_spell_enums()
    for spellInfo in spell_enums:
        if spellInfo.spell_enum == spell_vacant:
            return 0
    return 1

def LevelupSpellsFinalize(obj, classLvlNew = -1):
    spEnums = char_editor.get_spell_enums()
    char_editor.spell_known_add(spEnums) # internally takes care of duplicates and the labels/vacant slots
    return 0
