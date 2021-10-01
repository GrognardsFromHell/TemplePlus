from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions
import char_class_utils

###################################################

def GetConditionName():
    return "Dragon Disciple"

def GetSpellCasterConditionName():
    return "Dragon Disciple Spellcasting"
    
print "Registering " + GetConditionName()

classEnum = stat_level_dragon_disciple
classSpecModule = __import__('class023_dragon_disciple')

selectHeritageId = 2301
breathWeaponEnum = 2302
toggleFlyingId = 2303

#Dict to handle Dragon Disciple Heritage
#Can easily be expanded by adding a new type of heritage to the dict
#[Colour, ElementType, Breath Weapon Shape (1 = Cone, 2 = Line)]
#If a new element type would be added e.g. sonic or negative,
#this also would be needed to add in both spells and in the partsys
dictDragonHeritage = {
1: ["Black", D20DT_ACID, 2],
2: ["Blue", D20DT_ELECTRICITY, 2],
3: ["Green", D20DT_ACID, 1],
4: ["Red", D20DT_FIRE, 1],
5: ["White", D20DT_COLD, 1],
6: ["Brass", D20DT_FIRE, 2],
7: ["Bronze", D20DT_ELECTRICITY, 2],
8: ["Copper", D20DT_ACID, 2],
9: ["Gold", D20DT_FIRE, 1],
10: ["Silver", D20DT_COLD, 1]
}
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

##### Dragon Disciple Class Features #####

###Handle Heritage
def selectHeritageRadial(attachee, args, evt_obj):
    if not args.get_arg(0):
        radialSelectHeritageParent = tpdp.RadialMenuEntryParent("Select Dragon Disciple Heritage")
        radialSelectHeritageParentId = radialSelectHeritageParent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
        for key in dictDragonHeritage.keys():
            dragonColour = dictDragonHeritage[key][0]
            radialHeritageColourId = tpdp.RadialMenuEntryPythonAction("{} Dragon Heritage".format(dragonColour), D20A_PYTHON_ACTION, selectHeritageId, key, "TAG_CLASS_FEATURES_DRAGON_DISCIPLES_HERITAGE")
            radialHeritageColourId.add_as_child(attachee, radialSelectHeritageParentId)
    return 0

def setHeritage(attachee, args, evt_obj):
    chosenHeritage = evt_obj.d20a.data1
    print "Selected Heritage: {}".format(chosenHeritage)
    args.set_arg(0, chosenHeritage)
    #Visual Feedback for selected Heritage
    heritageColour = dictDragonHeritage[chosenHeritage][0]
    attachee.float_text_line("{} Heritage chosen".format(heritageColour))
    return 0

def querySelectedHeritage(attachee, args, evt_obj):
    heritage = args.get_arg(0)
    evt_obj.return_val = heritage
    return 0

def queryHeritageElementType(attachee, args, evt_obj):
    heritage = args.get_arg(0)
    elementType = dictDragonHeritage[heritage][1]
    evt_obj.return_val = elementType
    return 0

def queryHeritageBreathWeaponType(attachee, args, evt_obj):
    heritage = args.get_arg(0)
    evt_obj.return_val = dictDragonHeritage[heritage][2]
    return 0

def initialHeritageValue(attachee, args, evt_obj):
    args.set_arg(0, 0)
    return 0

dragonHeritage = PythonModifier("Dragon Disciple Heritage", 3) #heritage, empty, empty
dragonHeritage.MapToFeat("Dragon Disciple Heritage")
dragonHeritage.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, selectHeritageRadial, ())
dragonHeritage.AddHook(ET_OnD20PythonActionPerform, selectHeritageId, setHeritage, ())
dragonHeritage.AddHook(ET_OnD20PythonQuery, "PQ_Dragon_Disciple_Selected_Heritage", querySelectedHeritage, ())
dragonHeritage.AddHook(ET_OnD20PythonQuery, "PQ_Dragon_Disciple_Element_Type", queryHeritageElementType, ())
dragonHeritage.AddHook(ET_OnD20PythonQuery, "PQ_Dragon_Disciple_Breath_Weapon_Type", queryHeritageBreathWeaponType, ())
dragonHeritage.AddHook(ET_OnConditionAdd, EK_NONE, initialHeritageValue, ())


### AC Bonus
def naturalArmorACBonus(attachee, args, evt_obj):
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
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Dragon Disciple Natural Armor~[TAG_CLASS_FEATURES_DRAGON_DISCIPLES_NATURAL_ARMOR_INCREASE]")
    return 0

naturalArmorInc = PythonModifier("Dragon Disciple Natural Armor", 0)
naturalArmorInc.MapToFeat("Dragon Disciple Natural Armor")
naturalArmorInc.AddHook(ET_OnGetAC, EK_NONE, naturalArmorACBonus, ())

### Ability Bonus
#def OnGetAbilityScore(attachee, args, evt_obj):
    #statType = args.get_param(0)
#    lvl = attachee.stat_level_get(classEnum)
#    statMod = args.get_param(1)
#    
#    newValue = statMod + evt_obj.bonus_list.get_sum()
#    if (newValue < 3): # ensure minimum stat of 3
#        statMod = 3-newValue
#    evt_obj.bonus_list.add(statMod, 0, 139)
#    return 0

#classSpecObj.AddHook(ET_OnAbilityScoreLevel, EK_STAT_STRENGTH, OnGetAbilityScore, ())


def onGetAbilityScoreStr(attachee, args, evt_obj):
    level = attachee.stat_level_get(classEnum)
    if level < 2:
        return 0
    elif level < 4:
        bonusValue = 2
    elif level < 10:
        bonusValue = 4
    else:
        bonusValue = 8
    bonusType = 0 #ID 0 = Untyped(stacking)
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Dragon Disciple Ability Boost~[TAG_CLASS_FEATURES_DRAGON_DISCIPLES_ABILITY_BOOST]")
    return 0

def onGetAbilityScoreCon(attachee, args, evt_obj):
    level = attachee.stat_level_get(classEnum)
    if level >= 6:
        bonusValue = 2
        bonusType = 0 #ID = 0 Untyped(stacking)
        evt_obj.bonus_list.add(bonusValue, bonusType, "~Dragon Disciple Ability Boost~[TAG_CLASS_FEATURES_DRAGON_DISCIPLES_ABILITY_BOOST]")
    return 0

def onGetAbilityScoreInt(attachee, args, evt_obj):
    level = attachee.stat_level_get(classEnum)
    if level >= 8:
        bonusValue = 2
        bonusType = 0 #ID = 0 Untyped(stacking)
        evt_obj.bonus_list.add(bonusValue, bonusType, "~Dragon Disciple Ability Boost~[TAG_CLASS_FEATURES_DRAGON_DISCIPLES_ABILITY_BOOST]")
    return 0

def onGetAbilityScoreCha(attachee, args, evt_obj):
    level = attachee.stat_level_get(classEnum)
    if level >= 10:
        bonusValue = 2
        bonusType = 0 #ID = 0 Untyped(stacking)
        evt_obj.bonus_list.add(bonusValue, bonusType, "~Dragon Disciple Ability Boost~[TAG_CLASS_FEATURES_DRAGON_DISCIPLES_ABILITY_BOOST]")
    return 0

classSpecObj.AddHook(ET_OnAbilityScoreLevel, EK_STAT_STRENGTH, onGetAbilityScoreStr, ())
classSpecObj.AddHook(ET_OnAbilityScoreLevel, EK_STAT_CONSTITUTION, onGetAbilityScoreCon, ())
classSpecObj.AddHook(ET_OnAbilityScoreLevel, EK_STAT_INTELLIGENCE, onGetAbilityScoreInt, ())
classSpecObj.AddHook(ET_OnAbilityScoreLevel, EK_STAT_CHARISMA, onGetAbilityScoreCha, ())

### Claws and Bite

### Breath Weapon
def breathWeaponRadial(attachee, args, evt_obj):
    chargesLeft = args.get_arg(0)
    maxCharges = args.get_arg(1)
    #I display the heritage colour in the Breath Weapon Radial
    #So the player gets a visual feedback, which colour he did choose
    heritage = attachee.d20_query("PQ_Dragon_Disciple_Selected_Heritage")
    dragonColour = dictDragonHeritage[heritage][0]
    breathWeaponShape = attachee.d20_query("PQ_Dragon_Disciple_Breath_Weapon_Type")
    spellEnum = spell_dragon_disciple_cone_breath if breathWeaponShape == 1 else spell_dragon_disciple_line_breath

    breathWeaponId = tpdp.RadialMenuEntryPythonAction("{} Breath Weapon {}/{}".format(dragonColour, chargesLeft, maxCharges), D20A_PYTHON_ACTION, breathWeaponEnum, spellEnum, "TAG_CLASS_FEATURES_DRAGON_DISCIPLES_BREATH_WEAPON")
    spellData = tpdp.D20SpellData(spellEnum)
    casterLevel = attachee.stat_level_get(classEnum)
    spellData.set_spell_class(classEnum)
    spellData.set_spell_level(casterLevel)
    breathWeaponId.set_spell_data(spellData)
    breathWeaponId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
    return 0

def checkBreathWeapon(attachee, args, evt_obj):
    if args.get_arg(0) < 1:
        evt_obj.return_val = AEC_OUT_OF_CHARGES
    return 0

def performBreathWeapon(attachee, args, evt_obj):
    target = evt_obj.d20a.target
    #attachee.turn_towards(target)
    if attachee.anim_goal_push_attack(target, 0, 0 ,0):
        new_anim_id = attachee.anim_goal_get_new_id()
        evt_obj.d20a.flags |= D20CAF_NEED_ANIM_COMPLETED
        evt_obj.d20a.anim_id = new_anim_id
    return 0

def frameBreathWeapon(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    spellPacket = currentSequence.spell_packet
    newSpellId = tpactions.get_new_spell_id()
    spellPacket.caster_level = attachee.stat_level_get(classEnum)
    tpactions.register_spell_cast(spellPacket, newSpellId)
    tpactions.trigger_spell_effect(newSpellId)

    #Reduce Breath Weapon Daily uses by 1
    chargesLeft = args.get_arg(0)
    chargesLeft -= 1
    args.set_arg(0, chargesLeft)
    return 0

def resetBreathWeapon(attachee, args, evt_obj):
    maxCharges = 1
    args.set_arg(0, maxCharges)
    args.set_arg(1, maxCharges)
    return 0

dragonDiscipleBreathWeapon = PythonModifier("Dragon Disciple Breath Weapon", 3) #chargesLeft, maxCharges, empty
dragonDiscipleBreathWeapon.MapToFeat("Dragon Disciple Breath Weapon")
dragonDiscipleBreathWeapon.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, breathWeaponRadial, ())
dragonDiscipleBreathWeapon.AddHook(ET_OnD20PythonActionCheck, breathWeaponEnum, checkBreathWeapon, ())
dragonDiscipleBreathWeapon.AddHook(ET_OnD20PythonActionPerform, breathWeaponEnum, performBreathWeapon, ())
dragonDiscipleBreathWeapon.AddHook(ET_OnD20PythonActionFrame, breathWeaponEnum, frameBreathWeapon, ())
dragonDiscipleBreathWeapon.AddHook(ET_OnConditionAdd, EK_NONE, resetBreathWeapon, ())
dragonDiscipleBreathWeapon.AddHook(ET_OnNewDay, EK_NEWDAY_REST, resetBreathWeapon, ())


### Blindsense
#dropped

###Wings
def addWings(attachee, args, evt_obj):
    #meshId = 
    #evt_obj.append(meshId)
    return 0

def wingsRadial(attachee, args, evt_obj):
    actionString = "Stop Flying" if attachee.d20_query("PQ_Is_Flying") else "Start Flying"
    radialWingsId = tpdp.RadialMenuEntryPythonAction("{}".format(actionString), D20A_PYTHON_ACTION, toggleFlyingId, 0, "TAG_CLASS_FEATURES_DRAGON_DISCIPLES_WINGS")
    radialWingsId.add_as_child(attachee, tpdp.RadialMenuStandardNode.Class)
    return 0

def resetWings(attachee, args, evt_obj):
    attachee.d20_signal("PS_Flying_End")
    return 0

def checkWingsIndoors(attachee, args, evt_obj):
    if not game.is_outdoor():
        evt_obj.return_val = AEC_ACTION_INVALID
    return 0

def toggleWings(attachee, args, evt_obj):
    isFlying = attachee.d20_query("PQ_Is_Flying")
    if isFlying:
        attachee.d20_send_signal("PS_Flying_End")
    else:
        attachee.condition_add_with_args("Flying Condition", 0, 0, 0)
    return 0

dragonDiscipleWings = PythonModifier("Dragon Disciple Dragon Wings", 3) #empty, empty, empty
dragonDiscipleWings.MapToFeat("Dragon Disciple Wings")
dragonDiscipleWings.AddHook(ET_OnAddMesh, EK_NONE, addWings, ())
dragonDiscipleWings.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, wingsRadial, ())
dragonDiscipleWings.AddHook(ET_OnD20PythonActionCheck, toggleFlyingId, checkWingsIndoors, ())
dragonDiscipleWings.AddHook(ET_OnD20PythonActionPerform, toggleFlyingId, toggleWings, ())
dragonDiscipleWings.AddHook(ET_OnNewDay, EK_NEWDAY_REST, resetWings, ())
dragonDiscipleWings.AddHook(ET_OnD20Signal, EK_S_Teleport_Reconnect, resetWings, ())


#### Dragon Apotheosis
#Not implemented:
#Half-dragon template
#Low-light vision
#60-foot darkvision
def sleepParalyzeImmunity(attachee, args, evt_obj):
    if evt_obj.is_modifier("sp-Sleep"):
        evt_obj.return_val = 0
        combatMesLine = 5059 #ID 5059: "Sleep Immunity"
        historyMesLine = 31 #ID 31: {[ACTOR] is immune to ~sleep~[TAG_SPELLS_SLEEP].}
        attachee.float_mesfile_line('mes\\combat.mes', combatMesLine, tf_red)
        game.create_history_from_pattern(historyMesLine, attachee, OBJ_HANDLE_NULL)
    elif evt_obj.is_modifier("Paralyzed"):
        evt_obj.return_val = 0
        attachee.float_text_line("Paralyze Immunity", tf_red)
        game.create_history_freeform("{} is immune to ~paralyze~[TAG_PARALYZED] effects\n\n".format(attachee.description))
    return 0

def elementImmunity(attachee, args, evt_obj):
    elementType = attachee.d20_query("PQ_Dragon_Disciple_Element_Type")
    damageMesLine = 132 #ID 132 in damage.mes is Immunity
    evt_obj.damage_packet.add_mod_factor(0.0, elementType, damageMesLine)
    return 0

dragonDiscipleApotheosis = PythonModifier("Dragon Disciple Dragon Apotheosis", 3) #empty, empty, empty
dragonDiscipleApotheosis.MapToFeat("Dragon Disciple Dragon Apotheosis")
dragonDiscipleApotheosis.AddHook(ET_OnConditionAddPre, EK_NONE, sleepParalyzeImmunity, ())
dragonDiscipleApotheosis.AddHook(ET_OnTakingDamage2, EK_NONE, elementImmunity, ())


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