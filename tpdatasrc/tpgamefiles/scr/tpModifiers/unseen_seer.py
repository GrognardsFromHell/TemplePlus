from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils
import char_editor
import functools

###################################################

def GetConditionName():
    return "Unseen Seer"

def GetSpellCasterConditionName():
    return "Unseen Seer Spellcasting"

print "Registering " + GetConditionName()

classEnum = stat_level_unseen_seer
classSpecModule = __import__('class089_unseen_seer')
###################################################

########## Python Action ID's ##########
unseenSeerAdvLearnEnum = 8901
########################################


#### standard callbacks - BAB and Save values
def OnGetToHitBonusBase(attachee, args, evt_obj):
    classLevel = attachee.stat_level_get(classEnum)
    babvalue = game.get_bab_for_class(classEnum, classLevel)
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

#### Unseen Seer Class Mechanics

def getClassFeatureTag(featName):
    return "TAG_CLASS_FEATURES_{}".format(featName.upper().replace(" ", "_"))

### Damage Bonus
#At 1st level, the extra damage you deal with your sneak attack, skirmish, or sudden strike ability increases by 1d6.
#If you have more than one of these abilities, only one ability gains this increase (choose each time you gain this benefit).
#Your sneak attack, skirmish, or sudden strike damage increases by another 1d6 at 4th level, 7th level, and 10th level.

def addDamageBonus(attachee, args, evt_obj):
    classLevel = attachee.stat_level_get(classEnum)
    bonusDice = int(1 + (classLevel - 1) / 3)
    if attachee.has_feat(feat_sneak_attack):
        evt_obj.return_val += bonusDice
    elif attachee.has_feat("Skirmish"):
        evt_obj.return_val += bonusDice
    elif attachee.has_feat("Sudden Strike"): #Sudden Strike is currently not implemented
        evt_obj.return_val += bonusDice
    return 0

classSpecObj.AddHook(ET_OnD20PythonQuery, "Sneak Attack Dice", addDamageBonus, ())
classSpecObj.AddHook(ET_OnD20PythonQuery, "Skirmish Additional Dice", addDamageBonus, ())
classSpecObj.AddHook(ET_OnD20PythonQuery, "PQ_Sudden_Strike_Additional_Dice", addDamageBonus, ())

### Advanced Learning
#At 2nd, 5th, and 8th level, you can add a new spell to your spellbook or list of spells known
#representing the result of personal study and experimentation. The spell must be a divination spell of a level no higher
#than that of the highest-level arcane spell you already know. The spell can be from any class's spell list (arcane or divine).
#Once a new spell is selected, it is forever added to your spell list and can be cast just like any other spell on your list.

#Currently Class Features are not availible yet, so Advanced Learning will be handled via Radial Workaround
#The Unseen Seer Advanced Learning differs from the implemented Advanced Learning in Temple+
#As the Unseen Seer class can learn spells from any class. The limitation is, that it must be a divination spell

def filterLearnableSpells(attachee, spell):
    spellEntry = tpdp.SpellEntry(spell.spell_enum)
    if not spellEntry.spell_school_enum == Divination:
        return False
    elif attachee.is_spell_known(spell.spell_enum):
        return False
    return True

def getAdvancedLearningList(attachee, spellClass, highestArcaneClass, maxSpellLevel):
    learnableSpellList = char_editor.get_learnable_spells(attachee, spellClass, maxSpellLevel)
    learnableSpellList = filter(functools.partial(filterLearnableSpells, attachee), learnableSpellList)
    for idx in range(0, len(learnableSpellList)):
        learnableSpellList[idx].set_casting_class(highestArcaneClass)
    return learnableSpellList

def getSpellClassList():
    return [stat_level_bard, stat_level_cleric, stat_level_druid, stat_level_paladin, stat_level_ranger, stat_level_wizard]

def advLearningAvailible(classLevel, advLearnedSpells):
    knownAdvLearnedSpells = {
    1: 0,
    2: 1,
    3: 1,
    4: 1,
    5: 2,
    6: 2,
    7: 2,
    8: 3,
    9: 3,
    10: 3
    }
    return True if knownAdvLearnedSpells[classLevel] > advLearnedSpells else False

def radialAdvancedLearning(attachee, args, evt_obj):
    advLearnedSpells = args.get_arg(1)
    classLevel = attachee.stat_level_get(classEnum)
    if not advLearningAvailible(classLevel, advLearnedSpells):
        return 0
    highestArcaneClass = attachee.highest_arcane_class
    maxSpellLevel = attachee.arcane_spell_level_can_cast()
    #Top Menu
    radialLabel = "Advanced Learning"
    radialTop = tpdp.RadialMenuEntryParent(radialLabel)
    radialTopId = radialTop.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
    #Build SubMenus
    for spellClass in getSpellClassList():
        #spellClass Top
        radialLabelClass = game.get_mesline("mes\\stat.mes", spellClass)
        radialSpellClass = tpdp.RadialMenuEntryParent(radialLabelClass)
        radialSpellClassId = radialSpellClass.add_as_child(attachee, radialTopId)
        #spellNodes
        spellNodeId = []
        for node in range(0, maxSpellLevel + 1):
            spellNode = tpdp.RadialMenuEntryParent("{}".format(node))
            spellNodeId.append(spellNode.add_as_child(attachee, radialSpellClassId))
        #Add Divination Spells from class
        classDivinationSpellList = getAdvancedLearningList(attachee, spellClass, highestArcaneClass, maxSpellLevel)
        for spell in classDivinationSpellList:
            spellEnum = spell.spell_enum
            spellData = tpdp.D20SpellData(spellEnum)
            spellData.set_spell_class(highestArcaneClass)
            spellData.set_spell_level(spell.spell_level)
            spellStore = spellData.get_spell_store()
            spellName = game.get_spell_mesline(spellEnum)
            spellHelpTag = "TAG_SPELLS_{}".format(spellName).upper().replace(" ", "_")
            radialId = tpdp.RadialMenuEntryPythonAction(spellStore, D20A_PYTHON_ACTION, unseenSeerAdvLearnEnum, spell.spell_enum, spellHelpTag)
            radialId.add_as_child(attachee, spellNodeId[spell.spell_level])
    return 0

def addAdvLearnedSpell(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(attachee, evt_obj.d20a.spell_data)
    spellEnum = evt_obj.d20a.data1 # spellPacket.spell_enum returns 0; seems like a bug
    spellName = game.get_spell_mesline(spellEnum)
    spellClassCode = spellPacket.spell_class
    spellLevel = spellPacket.spell_known_slot_level
    attachee.spell_known_add(spellEnum, spellClassCode, spellLevel)
    attachee.float_text_line("{} learned".format(spellName))
    advLearnedSpells = args.get_arg(1)
    advLearnedSpells += 1
    args.set_arg(1, advLearnedSpells)
    return 0

advancedLearningFeature = PythonModifier("Unseen Seer Advanced Learning", 3) #featEnum, advLearnedSpells, empty
advancedLearningFeature.MapToFeat("Unseen Seer Advanced Learning", feat_cond_arg2 = 0)
advancedLearningFeature.AddHook(ET_OnBuildRadialMenuEntry , EK_NONE, radialAdvancedLearning, ())
advancedLearningFeature.AddHook(ET_OnD20PythonActionPerform, unseenSeerAdvLearnEnum, addAdvLearnedSpell, ())


#ToDo

### Silent Spell
#At 2nd level, you gain Silent Spell as a bonus feat.

#Automatically granted by class

### Divination Spell Power
#At 3rd level, you gain a +1 bonus to your caster level when casting an arcane divination spell.
#This bonus improves to +2 at 6th level, and to +3 at 9th level.
#This benefit comes at a cost: Your caster level for all other arcane spells is reduced by 1 at 3rd level.
#This reduction becomes 2 at 6th level and becomes 3 at 9th level.

def getCasterLevelModification(attachee, args, evt_obj):
    classLevel = attachee.stat_level_get(classEnum)
    divinationModificator = int(classLevel / 3)
    if divinationModificator:
        spellPacket = evt_obj.get_spell_packet()
        spellEnum = spellPacket.spell_enum
        spellEntry = tpdp.SpellEntry(spellEnum)
        if spellEntry.spell_school_enum == Divination:
            evt_obj.return_val += divinationModificator
        else:
            evt_obj.return_val -= divinationModificator
    return 0

classSpecObj.AddHook(ET_OnGetCasterLevelMod, EK_NONE, getCasterLevelModification, ())

###Guarded Mind
#Any successful unseen seer must learn to protect herself from magic that would reveal her identity.
#At 5th level, you become protected by nondetection (as the spell, but with a permanent duration).
#For the purpose of divinations attempted against you, your caster level equals your character level.

#Skip for now

##### Spell casting

# configure the spell casting condition to hold the highest Arcane classs
def OnAddSpellCasting(attachee, args, evt_obj):
    #arg0 holds the arcane class
    if args.get_arg(0) == 0:
        args.set_arg(0, char_class_utils.GetHighestArcaneClass(attachee))
    return 0

# Extend caster level for base casting class
def OnGetBaseCasterLevel(attachee, args, evt_obj):
    class_extended_1 = args.get_arg(0)
    class_code = evt_obj.arg0
    if class_code != class_extended_1:
        if evt_obj.arg1 == 0: # arg1 != 0 means you're looking for this particular class's contribution
            return 0
    classLevel = attachee.stat_level_get(classEnum)
    evt_obj.bonus_list.add(classLevel, 0, 137)
    return 0

def OnSpellListExtensionGet(attachee, args, evt_obj):
    class_extended_1 = args.get_arg(0)
    class_code = evt_obj.arg0
    if class_code != class_extended_1:
        if evt_obj.arg1 == 0: # arg1 != 0 means you're looking for this particular class's contribution
            return 0
    classLevel = attachee.stat_level_get(classEnum)
    evt_obj.bonus_list.add(classLevel, 0, 137)
    return 0

def OnInitLevelupSpellSelection(attachee, args, evt_obj):
    if evt_obj.arg0 != classEnum:
        return 0
    #classLevel = attachee.stat_level_get(classEnum)
    class_extended_1 = args.get_arg(0)
    classSpecModule.InitSpellSelection(attachee, class_extended_1)
    return 0

def OnLevelupSpellsCheckComplete(attachee, args, evt_obj):
    if (evt_obj.arg0 != classEnum):
        return 0
    class_extended_1 = args.get_arg(0)
    if not classSpecModule.LevelupCheckSpells(attachee, class_extended_1):
        evt_obj.bonus_list.add(-1, 0, 137) # denotes incomplete spell selection
    return 1
    
def OnLevelupSpellsFinalize(attachee, args, evt_obj):
    if evt_obj.arg0 != classEnum:
        return 0
    #classLevel = attachee.stat_level_get(classEnum)
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