from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions
import char_class_utils
import heritage_feat_utils
import breath_weapon

###################################################

def GetConditionName():
    return "Dragon Disciple"

def GetSpellCasterConditionName():
    return "Dragon Disciple Spellcasting"
    
print "Registering " + GetConditionName()

classEnum = stat_level_dragon_disciple
classSpecModule = __import__('class023_dragon_disciple')

breathWeaponEnum = 2302
toggleFlyingId = 2303

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

### Draconic Heritage is handle by the Draconic Heritage Feat now

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
# ToDo!

### Breath Weapon

def getBreathWeaponTag():
    return "TAG_CLASS_FEATURES_DRAGON_DISCIPLES_BREATH_WEAPON"

def breathWeaponRadial(attachee, args, evt_obj):
    print "breathWeaponRadial Hook"
    breathWeaponCooldown = args.get_arg(2)
    if breathWeaponCooldown > -1:
        breathWeaponId = tpdp.RadialMenuEntryPythonAction("Breath Weapon Cooldown ({} round(s))".format(breathWeaponCooldown), D20A_PYTHON_ACTION, breathWeaponEnum, 0, "TAG_EXTRA_EXALATION")
    else:
        chargesLeft = args.get_arg(1)
        maxCharges = breath_weapon.getMaxCharges(attachee, args)
        heritage = attachee.d20_query("PQ_Selected_Draconic_Heritage")
        breathWeaponShape = heritage_feat_utils.getDraconicHeritageBreathShape(heritage)
        spellEnum = spell_dragon_diciple_cone_breath if breathWeaponShape == dragon_breath_shape_cone else spell_dragon_diciple_line_breath
        breathWeaponTag = getBreathWeaponTag()
        breathWeaponId = tpdp.RadialMenuEntryPythonAction("Breath Weapon ({}/{})".format(chargesLeft, maxCharges), D20A_PYTHON_ACTION, breathWeaponEnum, spellEnum, breathWeaponTag)
        spellData = tpdp.D20SpellData(spellEnum)
        spellData.set_spell_class(classEnum)
        spellData.set_spell_level(9) #Setting this to 9 here so, it passes globes of invulnerability, as they should not protect against Breath Weapons
        breathWeaponId.set_spell_data(spellData)
    breathWeaponId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
    return 0

def checkBreathWeapon(attachee, args, evt_obj):
    if args.get_arg(1) < 1:
        evt_obj.return_val = AEC_OUT_OF_CHARGES
    elif args.get_arg(2) > -1:
        evt_obj.return_val = AEC_ABILITY_ON_COOLDOWN 
    return 0

def performBreathWeapon(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    spellPacket = currentSequence.spell_packet
    newSpellId = tpactions.get_new_spell_id()
    spellPacket.caster_level += attachee.stat_level_get(classEnum)
    tpactions.register_spell_cast(spellPacket, newSpellId)
    currentSequence.spell_packet.spell_id = newSpellId

    if attachee.anim_goal_throw_spell_w_cast_anim(): # note: the animation goal has internal calls to trigger_spell_effect and the action frame
        new_anim_id = attachee.anim_goal_get_new_id()
        evt_obj.d20a.flags |= D20CAF_NEED_ANIM_COMPLETED
        evt_obj.d20a.anim_id = new_anim_id
    return 0

def frameBreathWeapon(attachee, args, evt_obj):
    breathWeaponTag = getBreathWeaponTag()
    genderString = "his" if attachee.stat_level_get(stat_gender) == 1 else "her"
    game.create_history_freeform("{} uses {} ~Breath Weapon~[{}]\n\n".format(attachee.description, genderString, breathWeaponTag))
    #Send Breath Weapon Used Signal
    breathWeaponId = args.get_arg(0)
    attachee.d20_send_signal("PS_Breath_Weapon_Used", breathWeaponId)
    return 0

dragonDiscipleBreathWeapon = breath_weapon.BreathWeaponModifier("Dragon Disciple Breath Weapon")
dragonDiscipleBreathWeapon.MapToFeat("Dragon Disciple Breath Weapon")
dragonDiscipleBreathWeapon.breathWeaponSetArgs(classEnum, 1) #1 = baseCharges of the Breath Weapon
dragonDiscipleBreathWeapon.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, breathWeaponRadial, ())
dragonDiscipleBreathWeapon.AddHook(ET_OnD20PythonActionCheck, breathWeaponEnum, checkBreathWeapon, ())
dragonDiscipleBreathWeapon.AddHook(ET_OnD20PythonActionPerform, breathWeaponEnum, performBreathWeapon, ())
dragonDiscipleBreathWeapon.AddHook(ET_OnD20PythonActionFrame, breathWeaponEnum, frameBreathWeapon, ())



### Blindsense
#dropped

###Wings
def addWings(attachee, args, evt_obj):
    meshId = 14201 #Darley Wings
    evt_obj.append(meshId)
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
    heritage = attachee.d20_query("PQ_Selected_Draconic_Heritage")
    elementType = heritage_feat_utils.getDraconicHeritageElement(heritage)
    damageMesLine = 132 #ID 132 in damage.mes is Immunity
    evt_obj.damage_packet.add_mod_factor(0.0, elementType, damageMesLine)
    return 0

dragonDiscipleApotheosis = PythonModifier("Dragon Disciple Dragon Apotheosis", 3) #empty, empty, empty
dragonDiscipleApotheosis.MapToFeat("Dragon Disciple Dragon Apotheosis")
dragonDiscipleApotheosis.AddHook(ET_OnConditionAddPre, EK_NONE, sleepParalyzeImmunity, ())
dragonDiscipleApotheosis.AddHook(ET_OnTakingDamage2, EK_NONE, elementImmunity, ())
