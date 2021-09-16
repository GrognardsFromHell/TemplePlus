from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

###################################################

def GetConditionName():
    return "Dragon Disciple"

def GetSpellCasterConditionName():
    return "Dragon Disciple Spellcasting"
    
print "Registering " + GetConditionName()

classEnum = stat_level_dragon_disciple
classSpecModule = __import__('class023_dragon_disciple')
###################################################


#### standard callbacks - BAB and Save values
def OnGetToHitBonusBase(attachee, args, evt_obj):
    classLvl = attachee.stat_level_get(classEnum)
    babvalue = game.get_bab_for_class(classEnum, classLvl)
    evt_obj.bonus_list.add(babvalue, 0, 137) # untyped, description: "Class"
    return 0

def OnGetSaveThrowFort(attachee, args, evt_obj):
    value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Fortitude)
    evt_obj.bonus_list.add(value, 0, 137)
    return 0

def OnGetSaveThrowReflex(attachee, args, evt_obj):
    value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Reflex)
    evt_obj.bonus_list.add(value, 0, 137)
    return 0

def OnGetSaveThrowWill(attachee, args, evt_obj):
    value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Will)
    evt_obj.bonus_list.add(value, 0, 137)
    return 0


classSpecObj = PythonModifier(GetConditionName(), 0)
classSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())


#region Spell casting

# configure the spell casting condition to hold the highest Arcane classs
def OnAddSpellCasting(attachee, args, evt_obj):
    #arg0 holds the arcane class
    if (args.get_arg(0) == 0):
        args.set_arg(0, char_class_utils.GetHighestArcaneClass(attachee))
    
    return 0

# Extend caster level for base casting class
def OnGetBaseCasterLevel(attachee, args, evt_obj):
    class_extended_1 = args.get_arg(0)
    class_code = evt_obj.arg0
    if (class_code != class_extended_1):
        if (evt_obj.arg1 == 0): # arg1 != 0 means you're looking for this particular class's contribution
            return 0
    classLvl = attachee.stat_level_get(classEnum)
    if classLvl > 1:
        evt_obj.bonus_list.add(classLvl - 1, 0, 137)
    return 0

def OnSpellListExtensionGet(attachee, args, evt_obj):
    class_extended_1 = args.get_arg(0)
    class_code = evt_obj.arg0
    if (class_code != class_extended_1):
        if (evt_obj.arg1 == 0): # arg1 != 0 means you're looking for this particular class's contribution
            return 0
    classLvl = attachee.stat_level_get(classEnum)
    if classLvl > 1:
        evt_obj.bonus_list.add(classLvl - 1, 0, 137)
    return 0

def OnInitLevelupSpellSelection(attachee, args, evt_obj):
    if (evt_obj.arg0 != classEnum):
        return 0
    classLvl = attachee.stat_level_get(classEnum)
    if (classLvl == 0):
        return 0
    class_extended_1 = args.get_arg(0)
    classSpecModule.InitSpellSelection(attachee, class_extended_1)
    return 0

def OnLevelupSpellsCheckComplete(attachee, args, evt_obj):
    if (evt_obj.arg0 != classEnum):
        return 0
    class_extended_1 = args.get_arg(0)
    if (not classSpecModule.LevelupCheckSpells(attachee, class_extended_1) ):
        evt_obj.bonus_list.add(-1, 0, 137) # denotes incomplete spell selection
    return 1
    
def OnLevelupSpellsFinalize(attachee, args, evt_obj):
    if (evt_obj.arg0 != classEnum):
        return 0
    classLvl = attachee.stat_level_get(classEnum)
    if (classLvl == 0):
        return 0
    class_extended_1 = args.get_arg(0)
    classSpecModule.LevelupSpellsFinalize(attachee, class_extended_1)
    return

spellCasterSpecObj = PythonModifier(GetSpellCasterConditionName(), 8)
spellCasterSpecObj.AddHook(ET_OnConditionAdd, EK_NONE, OnAddSpellCasting, ())
spellCasterSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
spellCasterSpecObj.AddHook(ET_OnSpellListExtensionGet, EK_NONE, OnSpellListExtensionGet, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Activate, OnInitLevelupSpellSelection, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Check_Complete, OnLevelupSpellsCheckComplete, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())

#endregion

##### Dragon Disciple Class Features #####

### AC Bonus
def NaturalArmorACBonus(attachee, args, evt_obj):
    classLevel = attachee.stat_level_get(classEnum)
    if classLevel == 1:
        bonusValue = 1
    elif classLevel < 7:
        bonusValue = 2
    elif classLevel < 10:
        bonusValue = 3
    else:
        bonusValue = 4
    bonusType = 0 #ID 0 = Stacking; Wrong Type as Touch Attacks should nullify this bonus
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Dragon Disciple Natural Armor~[TAG_CLASS_FEATURES_DRAGON_DISCIPILES_NATURAL_ARMOR_INCREASE]")
    return 0

naturalArmorInc = PythonModifier("Dragon Disciple Natural Armor", 0)
naturalArmorInc.MapToFeat("Dragon Disciple Natural Armor")
naturalArmorInc.AddHook(ET_OnGetAC, EK_NONE, NaturalArmorACBonus, ())

### Strength Bonus
def OnGetAbilityScore(attachee, args, evt_obj):
    #statType = args.get_param(0)
    lvl = attachee.stat_level_get(classEnum)
    statMod = args.get_param(1)
    
    newValue = statMod + evt_obj.bonus_list.get_sum()
    if (newValue < 3): # ensure minimum stat of 3
        statMod = 3-newValue
    evt_obj.bonus_list.add(statMod, 0, 139)
    return 0

classSpecObj.AddHook(ET_OnAbilityScoreLevel, EK_STAT_STRENGTH, OnGetAbilityScore, ())

### Breath Weapon


### Blindsense


#### Dragon Apotheosis
# At 10th level, a dragon disciple takes on the half-dragon template. His breath weapon reaches full strength (as noted above), 
# and he gains +4 to Strength and +2 to Charisma. His natural armor bonus increases to +4, and he acquires low-light vision, 
#60-foot darkvision, immunity to sleep and paralysis effects, and immunity to the energy type used by his breath weapon.
