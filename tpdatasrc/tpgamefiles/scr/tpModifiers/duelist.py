from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils
import d20_action_utils

###################################################

def GetConditionName():
	return "Duelist"

print "Registering " + GetConditionName()

classEnum = stat_level_duelist
preciseStrikeEnum = 2400
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


def IsArmorless( obj ):
	armor = obj.item_worn_at(5)
	if armor != OBJ_HANDLE_NULL:
		armorFlags = armor.obj_get_int(obj_f_armor_flags)
		if armorFlags != ARMOR_TYPE_NONE:
			return 0
	shield = obj.item_worn_at(11)
	if shield != OBJ_HANDLE_NULL:
		return 0
	return 1
	

def IsRangedWeapon( weap ):
	weapFlags = weap.obj_get_int(obj_f_weapon_flags)
	if (weapFlags & OWF_RANGED_WEAPON) == 0:
		return 0
	return 1
	
def CannyDefenseAcBonus(attachee, args, evt_obj):
	if not IsArmorless(attachee):
		return 0
	weap = attachee.item_worn_at(3)
	if weap == OBJ_HANDLE_NULL or IsRangedWeapon(weap):
		weap = attachee.item_worn_at(4)
		if weap == OBJ_HANDLE_NULL or IsRangedWeapon(weap):
			return 0
	duelistLvl = attachee.stat_level_get(classEnum)
	intScore = attachee.stat_level_get(stat_intelligence)
	intBonus = (intScore - 10)/2
	if intBonus <= 0:
		return
	if duelistLvl < intBonus:
		intBonus = duelistLvl
	evt_obj.bonus_list.modify(intBonus , 3, 104) # Dexterity bonus,  ~Class~[TAG_LEVEL_BONUSES]
	return 0

def ImprovedReactionInitBonus(attachee, args, evt_obj):
	duelistLvl = attachee.stat_level_get(classEnum)
	if duelistLvl < 2:
		return 0
	bonVal = 2
	if duelistLvl >= 8:
		bonVal = 4
	evt_obj.bonus_list.add(bonVal, 0, 137 ) # adds untyped bonus to initiative
	return 0

def EnhancedMobility(attachee, args, evt_obj):
	duelistLvl = attachee.stat_level_get(classEnum)
	if duelistLvl < 3:
		return 0
	if not IsArmorless(attachee):
		return 0
	if evt_obj.attack_packet.get_flags() & D20CAF_AOO_MOVEMENT:
		evt_obj.bonus_list.add(4, 8, 137 ) # adds +4 dodge bonus
	return 0

def GraceReflexBonus(attachee, args, evt_obj):
	duelistLvl = attachee.stat_level_get(classEnum)
	if duelistLvl < 4:
		return 0
	if not IsArmorless(attachee):
		return 0
	evt_obj.bonus_list.add(2, 34, 137) # Competence bonus
	return 0
	

# def PreciseStrikeRadial(attachee, args, evt_obj):
	# duelistLvl = attachee.stat_level_get(classEnum)
	# if (duelistLvl < 5):
		# return 0
	## add radial menu action Precise Strike
	# radialAction = tpdp.RadialMenuEntryPythonAction(-1, D20A_PYTHON_ACTION, preciseStrikeEnum, 0,  "TAG_INTERFACE_HELP")
	# radialParentId = radialAction.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
	# return 0

# def OnPreciseStrikeCheck(attachee, args, evt_obj):
	# if (not IsUsingLightOrOneHandedPiercing(attachee)):
		# evt_obj.return_val = AEC_WRONG_WEAPON_TYPE
		# return 0
	
	# tgt = evt_obj.d20a.target
	# stdChk = ActionCheckTargetStdAtk(attachee, tgt)
	# if (stdChk != AEC_OK):
		# evt_obj.return_val = stdChk
	
	# return 0

# def OnPreciseStrikePerform(attachee, args, evt_obj):
	
	# print "I performed!"
	
	# return 0
preciseStrikeString = "Precise Strike"
def PreciseStrikeDamageBonus(attachee, args, evt_obj):
	duelistLvl = attachee.stat_level_get(classEnum)
	if duelistLvl < 5:
		return 0
	# check if attacking with one weapon and without a shield
	if (attachee.item_worn_at(4) != OBJ_HANDLE_NULL and attachee.item_worn_at(3) != OBJ_HANDLE_NULL) or attachee.item_worn_at(11) != OBJ_HANDLE_NULL:
		return 0
	# check if light or one handed piercing
	if not IsUsingLightOrOneHandedPiercing(attachee):
		return 0
	tgt = evt_obj.attack_packet.target
	if tgt == OBJ_HANDLE_NULL: # shouldn't happen but better be safe
		return 0
	if tgt.d20_query(Q_Critter_Is_Immune_Critical_Hits):
		return 0
	damage_dice = dice_new('1d6')
	if duelistLvl >= 10:
		damage_dice.number = 2
	
	evt_obj.damage_packet.add_dice(damage_dice, -1, 127 )
	return 0

def ElaborateParry(attachee, args, evt_obj):
	duelistLvl = attachee.stat_level_get(classEnum)
	if duelistLvl < 7:
		return 0
		
	if not attachee.d20_query(Q_FightingDefensively): # this also covers Total Defense
		return 0
	
	evt_obj.bonus_list.add(duelistLvl , 8, 137) # Dodge bonus,  ~Class~[TAG_LEVEL_BONUSES]
	return 0

def IsUsingLightOrOneHandedPiercing( obj ):
	weap = obj.item_worn_at(3)
	offhand = obj.item_worn_at(4)
	if weap == OBJ_HANDLE_NULL and offhand == OBJ_HANDLE_NULL:
		return 0
	if weap == OBJ_HANDLE_NULL:
		weap = offhand
		offhand = OBJ_HANDLE_NULL
	if IsWeaponLightOrOneHandedPiercing(obj, weap):
		return 1
	# check the offhand
	if offhand != OBJ_HANDLE_NULL:
		if IsWeaponLightOrOneHandedPiercing(obj, offhand):
			return 1
	return 0
	
def IsWeaponLightOrOneHandedPiercing( obj, weap):
	# truth table
	# nor. | enlarged |  return    
	#	0		x			1 			assume un-enlarged state
	#	1		0			1			shouldn't be possible... unless it's actually reduce person (I don't really care about that)
	#	1		1		is_piercing
	#	1 		2		is_piercing
	# 	2		x 			0           
	# 	3		x 			0           
	normalWieldType = obj.get_wield_type(weap, 1) # "normal" means weapon is not enlarged
	if normalWieldType >= 2: # two handed or unwieldable
		return 0
	if normalWieldType == 0:
		return 1
	
	# otherwise if the weapon is also enlarged; 
	wieldType = obj.get_wield_type(weap, 0)
	if wieldType == 0:
		return 1
	# weapon is not light, but is one handed - check if piercing
	attackType = weap.obj_get_int(obj_f_weapon_attacktype)
	if attackType == D20DT_PIERCING: # should be strictly piercing from what I understand (supposed to be rapier-like)
		return 1
	return 0
	
	
def DuelistDeflectArrows(attachee, args, evt_obj):
	duelistLvl = attachee.stat_level_get(classEnum)
	if duelistLvl < 9:
		return 0
	offendingWeapon = evt_obj.attack_packet.get_weapon_used()
	if offendingWeapon == OBJ_HANDLE_NULL:
		return 0
	if not (evt_obj.attack_packet.get_flags() & D20CAF_RANGED):
		return 0
	# check if attacker visible
	attacker = evt_obj.attack_packet.attacker
	if attacker == OBJ_HANDLE_NULL:
		return 0
	if attacker.d20_query(Q_Critter_Is_Invisible) and not attachee.d20_query(Q_Critter_Can_See_Invisible):
		return 0
	if attachee.d20_query(Q_Critter_Is_Blinded):
		return 0
	# check flatfooted
	if attachee.d20_query(Q_Flatfooted):
		return 0
	# check light weapon or one handed piercing
	if not IsUsingLightOrOneHandedPiercing(attachee):
		return 0
	
	atkflags = evt_obj.attack_packet.get_flags()
	atkflags |= D20CAF_DEFLECT_ARROWS
	atkflags &= ~(D20CAF_HIT | D20CAF_CRITICAL)
	evt_obj.attack_packet.set_flags(atkflags)

	return 0
	
classSpecObj = PythonModifier(GetConditionName(), 0)
classSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())
classSpecObj.AddHook(ET_OnGetAC, EK_NONE, CannyDefenseAcBonus, ())
classSpecObj.AddHook(ET_OnGetAC, EK_NONE, EnhancedMobility, ())
classSpecObj.AddHook(ET_OnGetAC, EK_NONE, ElaborateParry, ())
classSpecObj.AddHook(ET_OnGetInitiativeMod, EK_NONE, ImprovedReactionInitBonus, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, GraceReflexBonus, ())
classSpecObj.AddHook(ET_OnDealingDamage, EK_NONE, PreciseStrikeDamageBonus, ())
classSpecObj.AddHook(ET_OnDeflectArrows, EK_NONE, DuelistDeflectArrows, ())
