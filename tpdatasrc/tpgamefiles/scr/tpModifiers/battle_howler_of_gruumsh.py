from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

###################################################

def GetConditionName():
    return "Battle Howler of Gruumsh"
    
def GetSpellCasterConditionName():
    return "Battle Howler of Gruumsh Spellcasting"

print "Registering " + GetSpellCasterConditionName()

classEnum = stat_level_battle_howler_of_gruumsh
classSpecModule = __import__('class086_battle_howler_of_gruumsh')
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

### Battle Howler Bardic Music Feature
def queryHowlerBonusMusic(attachee, args, evt_obj):
    classLevel = attachee.stat_level_get(classEnum)
    evt_obj.return_val += classLevel
    return 0

classSpecObj.AddHook(ET_OnD20PythonQuery, "Bardic Music Bonus Levels", queryHowlerBonusMusic, ())

### Battle Howler Rage Feature
#The feature grants Rage 1/Day (2 at level 5) or stacks with exsting rage feature
#Rage grants 1 charge by itself
def queryHowlerRageDailyUses(attachee, args, evt_obj):
    classLevel = attachee.stat_level_get(classEnum)
    barbarianLevel = attachee.stat_level_get(stat_level_barbarian)
    if classLevel < 5:
        evt_obj.return_val += 0
    else:
        evt_obj.return_val += 1
    if barbarianLevel > 0 and classLevel > 1:
        evt_obj.return_val += 1
    return 0

classSpecObj.AddHook(ET_OnD20PythonQuery, "PQ_Get_Extra_Barbarian_Rage_Charges", queryHowlerRageDailyUses, ())


### Battle Howler War Cry
#When a battle howler uses the inspire courage ability of bardic music,
#it affects all allies within 60 feet and the morale bonuses it provides are increased by +1.

#Bardic Music has normally a range of 30 feet, so War Cry heavily modifies Inspire Courage


### Battle Howler Howling Rage
#At 4th level, a battle howler can use the bardic music abilities
#inspire courage, inspire greatness, and inspire heroics while raging,
#provided she has access to them.

#At the moment you can sing while raged, so the class ability is useless.
#This will be reworked once bardic music is redone in Python

### Spell casting
# configure the spell casting condition to hold the bard class
def OnAddSpellCasting(attachee, args, evt_obj):
    # arg0 holds the arcane class
    if args.get_arg(0) == 0:
        args.set_arg(0, stat_level_bard)

    return 0


# Extend caster level for base casting class
def OnGetBaseCasterLevel(attachee, args, evt_obj):
    class_extended_1 = args.get_arg(0)
    class_code = evt_obj.arg0
    if class_code != class_extended_1:
        if evt_obj.arg1 == 0:  # arg1 != 0 means you're looking for this particular class's contribution
            return 0
    classLvl = attachee.stat_level_get(classEnum)
    evt_obj.bonus_list.add(classLvl , 0, 137)
    return 0


def OnSpellListExtensionGet(attachee, args, evt_obj):
    class_extended_1 = args.get_arg(0)
    class_code = evt_obj.arg0
    if class_code != class_extended_1:
        if evt_obj.arg1 == 0:  # arg1 != 0 means you're looking for this particular class's contribution
            return 0
    classLvl = attachee.stat_level_get(classEnum)
    evt_obj.bonus_list.add(classLvl, 0, 137)
    return 0


def OnInitLevelupSpellSelection(attachee, args, evt_obj):
    if evt_obj.arg0 != classEnum:
        return 0
    class_extended_1 = args.get_arg(0)
    classSpecModule.InitSpellSelection(attachee, class_extended_1)
    return 0


def OnLevelupSpellsCheckComplete(attachee, args, evt_obj):
    if evt_obj.arg0 != classEnum:
        return 0
    class_extended_1 = args.get_arg(0)
    if not classSpecModule.LevelupCheckSpells(attachee, class_extended_1):
        evt_obj.bonus_list.add(-1, 0, 137)  # denotes incomplete spell selection
    return 1


def OnLevelupSpellsFinalize(attachee, args, evt_obj):
    if evt_obj.arg0 != classEnum:
        return 0
    class_extended_1 = args.get_arg(0)
    classSpecModule.LevelupSpellsFinalize(attachee, class_extended_1)
    return 0


spellCasterSpecObj = PythonModifier(GetSpellCasterConditionName(), 8)
spellCasterSpecObj.AddHook(ET_OnConditionAdd, EK_NONE, OnAddSpellCasting, ())
spellCasterSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
spellCasterSpecObj.AddHook(ET_OnSpellListExtensionGet, EK_NONE, OnSpellListExtensionGet, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Activate, OnInitLevelupSpellSelection, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Check_Complete, OnLevelupSpellsCheckComplete, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())



#def BardicMusicEnd(attachee, args, evt_obj):
#    partsysId = args.get_arg(5)
#    if partsysId:
#        game.particles_kill(partsysId)
#        args.set_arg(5,0)
#    args.set_arg(1,0) # music type currently playing
#    args.set_arg(2,0) # rounds music played
#    args.set_arg(3,0) # target handle
#    args.set_arg(4,0) # target handle upper
#    return 0

#bardicMus = PythonModifier()
#bardicMus.ExtendExisting("Bardic Music")
#bardicMus.AddHook(ET_OnD20PythonQuery, "Bardic Music Type", GetBardicMusicType, ())
#bardicMus.AddHook(ET_OnD20PythonSignal, "Bardic Music End", BardicMusicEnd, ())
