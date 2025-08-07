from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils
import math

###################################################

def GetConditionName():
	return "Scout"

classEnum = stat_level_scout
classSpecModule = __import__("class046_scout")
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
	
def OnQueryFindTraps(attachee, args, evt_obj):
	evt_obj.return_val = 1
	return 0

classSpecObj = PythonModifier(GetConditionName(), 0)
classSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())
classSpecObj.AddHook(ET_OnD20Query, EK_Q_Critter_Can_Find_Traps, OnQueryFindTraps, ())

## Scout Specific Feats

# Skirmish

# Global variables for keeping track of the location of the scout at the beginning of the round
# They do not need to be persistent except during a scout's turn
start_position_x = 0.0
start_position_y = 0.0
start_location = long()

#Checks for a load greater than light or armor greater than light (to enable skrimish, battle fortitude and fast movement)
def ScoutEncumberedCheck(obj):
	#Light armor or no armor
	armor = obj.item_worn_at(5)
	if armor != OBJ_HANDLE_NULL:
		armorFlags = armor.obj_get_int(obj_f_armor_flags)
		if (armorFlags != ARMOR_TYPE_LIGHT) and (armorFlags !=  ARMOR_TYPE_NONE):
			return 1
			
	#No heavy or medium load
	HeavyLoad = obj.d20_query(Q_Critter_Is_Encumbered_Heavy)
	if HeavyLoad:
		return 1
	MediumLoad = obj.d20_query(Q_Critter_Is_Encumbered_Medium)
	if MediumLoad:
		return 1
		
	return 0

#Calculate the skrimish bonus for the scout
def GetSkirmishACBonus(obj):
	scoutLevel = float(obj.stat_level_get(stat_level_scout))
	scoutBonusLevels = obj.d20_query("Skrimish Level Bonus")
	scoutLevel = scoutLevel + scoutBonusLevels
	bonusValue = int((scoutLevel + 1.0) / 4.0)
	skirmishExtraAC = obj.d20_query("Skirmish Additional AC")
	bonusValue += skirmishExtraAC
	return bonusValue
	
def GetSkirmishDamageDice(obj):
	scoutLevel = float(obj.stat_level_get(stat_level_scout))
	scoutBonusLevels = obj.d20_query("Skrimish Level Bonus")
	scoutLevel = scoutLevel + scoutBonusLevels
	bonusValue = int((scoutLevel - 1)/ 4.0 + 1)
	skirmishExtraDice = obj.d20_query("Skirmish Additional Dice")
	bonusValue += skirmishExtraDice
	bonusDice = str(bonusValue) + "d6"
	return bonusDice

#Determine if skrimish is enabled based on if the scout has moved 10 feet
def SkirmishEnabled(distance):
	returnValue = 0
	if distance >= 10:
		returnValue = 1
	return returnValue

def SkirmishTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not SkirmishEnabled(args.get_arg(0)):
		return 0
	
	#Scout must not be encumbered
	if ScoutEncumberedCheck(attachee):
		return 0

	# Set the tooltip
	evt_obj.append("Skirmish")

	return 0

def SkirmishEffectTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not SkirmishEnabled(args.get_arg(0)):
		return 0
	
	#Scout must not be encumbered
	if ScoutEncumberedCheck(attachee):
		return 0
	
	#Generate the tooltip
	tipString = "(" + GetSkirmishDamageDice(attachee)
	skirmishACBonus = GetSkirmishACBonus(attachee)
	if skirmishACBonus > 0:
		tipString = tipString + ",+" + str(skirmishACBonus) + "AC"
	tipString = tipString + ")"
	
	evt_obj.append(tpdp.hash("SCOUT_SKIRMISH"), -2, tipString)
	
	return 0

def ScoutMovedDistance(attachee, args, evt_obj):
	#Keep track of how far the scout as moved from their initial position (not total distance moved)
	global start_location
	global start_position_x
	global start_position_y
	
	#The distance needs to location at the beginning of the round needs to be adjusted by the radius (which is in inches)
	moveDistance = int(attachee.distance_to(start_location, start_position_x, start_position_y) + (attachee.radius / 12.0))
	args.set_arg(0, moveDistance)
	return 0

def SkirmishReset(attachee, args, evt_obj):
	global start_location
	global start_position_x
	global start_position_y

	#Save the initial position for the scout and the distance moved for the round
	start_position_x = attachee.off_x
	start_position_y = attachee.off_y
	start_location = attachee.location

	#Zero out the total distance moved from the start position
	args.set_arg(0, 0)
	return 0
	
def SkirmishAdd(attachee, args, evt_obj):
	args.set_arg(0, 0)  #Zero out the total distance moved from the start position
	return 0

def SkirmishAcBonus(attachee, args, evt_obj):
	# not active, do nothing
	if not SkirmishEnabled(args.get_arg(0)):
		return 0
	
	#Scout must not be encumbered
	if ScoutEncumberedCheck(attachee):
		return 0
	
	bonusValue = GetSkirmishACBonus(attachee)
	
	if (bonusValue > 0):
		evt_obj.bonus_list.add(bonusValue, 34, "Skirmish")  # Compitence Bonus
	return 0

	
def SkirmishDamageBonus(attachee, args, evt_obj):
	
	#Scout must not be encumbered
	if ScoutEncumberedCheck(attachee):
		return 0
	
	#imprecise attacks cannot get skirmish
	attackFlags = evt_obj.attack_packet.get_flags()
	if attackFlags & D20CAF_NO_PRECISION_DAMAGE:
		return 0
	
	skirmishEnabled = 0
	
	#Check if skrimish has been enabled
	if SkirmishEnabled(args.get_arg(0)):
		skirmishEnabled = 1
	
	target = evt_obj.attack_packet.target
	
	#Disable if too far away (30 is the standard limit)
	skirmishRange = 30 + attachee.d20_query("Skirmish Range Increase")
	if attachee.distance_to(target) > skirmishRange:
		skirmishEnabled = 0
	
	#Disable skirmish if the target can't be seen
	if not attachee.can_sense(target):
		skirmishEnabled = 0
	
	#Check if sneak attack is turned on for criticals and it was a critical hit (this counts for skrimish too)
	sneakAttackOnCritical = attachee.d20_query("Sneak Attack Critical")
	
	if sneakAttackOnCritical:
		if attackFlags & D20CAF_CRITICAL:
			skirmishEnabled = 1
			
	if not skirmishEnabled:
		return 0
		
	#Check for immunity to skirmish (same as immunity to sneak attack)
	NoSneakAttack = target.d20_query(Q_Critter_Is_Immune_Critical_Hits)
	
	if NoSneakAttack:
		return 0
	
	#Damage Bonus:  1- 1d5, 5- 2d6, 9-3d6, 13- 4d6, 17 - 5d6
	damageString = GetSkirmishDamageDice(attachee)
	damage_dice = dice_new(damageString)
	
	evt_obj.damage_packet.add_dice(damage_dice, -1, 127)
	return 0
	
scoutSkirmish = PythonModifier("Skirmish", 4) #Distance Moved, Spare, Spare, Spare
scoutSkirmish.MapToFeat("Skirmish")
scoutSkirmish.AddHook(ET_OnDealingDamage, EK_NONE, SkirmishDamageBonus, ())
scoutSkirmish.AddHook(ET_OnDealingDamageWeaponlikeSpell, EK_NONE, SkirmishDamageBonus, ())
scoutSkirmish.AddHook(ET_OnGetAC, EK_NONE, SkirmishAcBonus, ())
scoutSkirmish.AddHook(ET_OnBeginRound, EK_NONE, SkirmishReset, ())
scoutSkirmish.AddHook(ET_OnConditionAdd, EK_NONE, SkirmishAdd, ())
scoutSkirmish.AddHook(ET_OnD20Signal, EK_S_Combat_Critter_Moved, ScoutMovedDistance, ())
scoutSkirmish.AddHook(ET_OnGetTooltip, EK_NONE, SkirmishTooltip, ())
scoutSkirmish.AddHook(ET_OnGetEffectTooltip, EK_NONE, SkirmishEffectTooltip, ())

# Battle Fortitude
def GetBattleFortitudeBonus(obj):
	scoutLevel = obj.stat_level_get(stat_level_scout)
	
	if scoutLevel < 2:
		bonusValue = 0
	elif scoutLevel < 11:
		bonusValue = 1
	elif scoutLevel < 20:
		bonusValue = 2
	else:
		bonusValue = 3
	
	return bonusValue
	
def BattleFortitudeInitBonus(attachee, args, evt_obj):

	#Scout must not be encumbered
	if ScoutEncumberedCheck(attachee):
		return 0

	bonusValue = GetBattleFortitudeBonus(attachee)
	
	#Add the Value
	if bonusValue > 0:
		evt_obj.bonus_list.add(bonusValue, 34, "Battle Fortitude" ) # Competence bonus to initiative
	
	return 0
	
def BattleFortitudeFortSaveBonus(attachee, args, evt_obj):

	#Scout must not be encumbered
	if ScoutEncumberedCheck(attachee):
		return 0

	bonusValue = GetBattleFortitudeBonus(attachee)
	
	if bonusValue > 0:
		evt_obj.bonus_list.add(bonusValue, 34, "Battle Fortitude") # Competence bonus
	
	return 0
	
scoutBattleFortitude = PythonModifier("Battle Fortitude", 2) #Spare, Spare
scoutBattleFortitude.MapToFeat("Battle Fortitude")
scoutBattleFortitude.AddHook(ET_OnGetInitiativeMod, EK_NONE, BattleFortitudeInitBonus, ())
scoutBattleFortitude.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, BattleFortitudeFortSaveBonus, ())

# Fast Movement Scout
def FastMovementScoutBonus(attachee, args, evt_obj):
	#Scout must not be encumbered
	if ScoutEncumberedCheck(attachee):
		return 0

	scoutLevel = attachee.stat_level_get(stat_level_scout)
	
	if scoutLevel < 3:
		bonusValue = 0
	elif scoutLevel < 11:
		bonusValue = 10
	else:
		bonusValue = 20
	
	#Enhancement bonus to movement
	if bonusValue > 0:
		evt_obj.bonus_list.add(bonusValue, 12, "Fast Movement Scout")  #Enhancement bonus to movement
	
	return 0

scoutFastMovement = PythonModifier("Fast Movement Scout", 2) #Spare, Spare
scoutFastMovement.MapToFeat("Fast Movement Scout")
scoutFastMovement.AddHook(ET_OnGetMoveSpeed, EK_NONE, FastMovementScoutBonus, ())

# Hide in Plain Sight Scout
def HideInPlainSightQueryScout(attachee, args, evt_obj):
	#Scout must not be encumbered
	if ScoutEncumberedCheck(attachee):
		return 0
		
	#Must be outdoors
	if not game.is_outdoor():
		return 0

	evt_obj.return_val = 1
	return 0

scoutHideInPlainSight = PythonModifier("Hide in Plain Sight Scout", 2) #Spare, Spare
scoutHideInPlainSight.MapToFeat("Hide in Plain Sight Scout")
scoutHideInPlainSight.AddHook(ET_OnD20PythonQuery, "Can Hide In Plain Sight", HideInPlainSightQueryScout, () )

# Free Movement Scout
def FreeMovementScout(attachee, args, evt_obj):
	#Scout must not be encumbered
	if ScoutEncumberedCheck(attachee):
		return 0

	evt_obj.return_val = 1
	return 0

scoutFreeMovement = PythonModifier("Free Movement", 2) #Spare, Spare
scoutFreeMovement.MapToFeat("Free Movement")
scoutFreeMovement.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Freedom_of_Movement, FreeMovementScout, ())

#Blindsight

#Returns the range of the blindsight ability.  It is queried by the engine.
def ScoutBlindsightRange(attachee, args, evt_obj):
	evt_obj.return_val = 30
	return 0

scoutBlindsight = PythonModifier("Blindsight", 2) #Spare, Spare
scoutBlindsight.MapToFeat("Blindsight")
scoutBlindsight.AddHook(ET_OnD20PythonQuery, "Blindsight Range", ScoutBlindsightRange, ())

