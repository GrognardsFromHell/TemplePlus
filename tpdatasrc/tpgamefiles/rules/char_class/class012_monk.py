from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName(): # used by API
    return "Monk"

def GetCategory():
    return "Core 3.5 Ed Classes"

def GetClassDefinitionFlags():
    return CDF_BaseClass | CDF_CoreClass

def GetClassHelpTopic():
    return "TAG_MONKS"

classEnum = stat_level_monk

###################################################

class_feats = {

1: (feat_simple_weapon_proficiency_monk, feat_improved_unarmed_strike, feat_stunning_attacks, feat_stunning_fist, feat_flurry_of_blows),
2: (feat_evasion,),
3: (feat_fast_movement, feat_still_mind),
4: (feat_ki_strike,),
5: (feat_purity_of_body,),
# 6: feat_improved_trip  - used to be the only choice, now you should choose either this or feat_improved_disarm
7: (feat_wholeness_of_body,),
9: (feat_improved_evasion,),
11: (feat_diamond_body,),
12: (feat_abundant_step,),
13: (feat_diamond_soul,),
15: (feat_quivering_palm,),
19: (feat_empty_body,),
20: (feat_perfect_self,)
}

class_skills = (skill_alchemy, skill_balance, skill_climb, skill_concentration, skill_craft, skill_diplomacy, skill_escape_artist, skill_hide, skill_jump, skill_knowledge_arcana, skill_knowledge_religion, skill_listen, skill_move_silently, skill_perform, skill_profession, skill_sense_motive, skill_spot, skill_swim, skill_tumble)

def GetHitDieType():
    return 8
    
def GetSkillPtsPerLevel():
    return 4
    
def GetBabProgression():
    return base_attack_bonus_type_semi_martial
    
def IsFortSaveFavored():
    return 1
    
def IsRefSaveFavored():
    return 1
    
def IsWillSaveFavored():
    return 1

def GetSpellListType():
    return spell_list_type_none

def IsClassSkill(skillEnum):
    return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
    return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
    return class_feats

def IsAlignmentCompatible( alignment):
    if (alignment & ALIGNMENT_LAWFUL) != 0:
        return 1
    return 0

def ObjMeetsPrereqs( obj ):
    char_classes = obj.char_classes
    if char_classes.count > 0:
        monkLevel = obj.stat_level_get(stat_level_monk)
        if monkLevel > 0 and char_classes[len(char_classes) - 1] != stat_level_monk:
            return 0
    return 1

# Levelup

def IsSelectingFeatsOnLevelup( obj ):
    newLvl = char_editor.stat_level_get( classEnum )
    if newLvl == 2:
        if obj.has_feat(feat_combat_reflexes) and obj.has_feat(feat_deflect_arrows) and obj.has_feat("Fiery Fist"):
            return 0
        return 1
    if newLvl == 6:
        if obj.has_feat(feat_improved_trip) and obj.has_feat(feat_improved_disarm):
            return 0
        return 1
    return 0

def LevelupGetBonusFeats(obj):
    newLvl = char_editor.stat_level_get( classEnum )
    bonus_feats = []
    if newLvl == 2:
        bonus_feats = [feat_combat_reflexes, feat_deflect_arrows]
        #I know, atm a monk has to have stunning fist
        #but maybe improved grapple will be added as a choice at first level
        if char_editor.has_feat(feat_stunning_fist):
            bonus_feats.append("Fiery Fist")
    if newLvl == 6:
        bonus_feats = [feat_improved_trip, feat_improved_disarm]
        #Fiery Ki Defense and Ki Blast also require to have Stunning Fist
        if char_editor.has_feat(feat_stunning_fist):
            bonus_feats.append("Fiery Ki Defense")
            bonus_feats.append("Ki Blast")
    bonFeatInfo = []
    for feat in bonus_feats:
        featInfo = char_editor.FeatInfo(feat)
        featInfo.feat_status_flags |= 4 # always pickable
        bonFeatInfo.append(featInfo)
    char_editor.set_bonus_feats(bonFeatInfo)
    return