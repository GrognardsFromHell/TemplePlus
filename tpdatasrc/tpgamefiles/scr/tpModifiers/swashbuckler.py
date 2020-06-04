from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils
import tpactions

###################################################

def GetConditionName():
	return "Swashbuckler"

print "Registering " + GetConditionName()

classEnum = stat_level_swashbuckler
classSpecModule = __import__('class049_swashbuckler')
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

#Checks for a load greater than light or armor greater than light (to enable various abilities)
def SwashbucklerEncumberedCheck(obj):
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
	
#Check if the weapons is usable with finesse 
def IsFinesseWeapon(creature, weapon):
	#Unarmed works
	if (weapon == OBJ_HANDLE_NULL):
		return 1

	#Ranged weapons don't work
	weapFlags = weapon.obj_get_int(obj_f_weapon_flags)
	if (weapFlags & OWF_RANGED_WEAPON):
		return 0
	
	#Light weapon works
	wieldType = creature.get_wield_type(weapon)
	if (wieldType == 0):
		return 1
		
	#Whip, rapier, spiked chain works
	WeaponType = weapon.get_weapon_type()
	if (WeaponType == wt_whip) or (WeaponType == wt_spike_chain) or (WeaponType == wt_rapier):
		return 1
		
	return 0
	

#Swashbuckler Abilities

# Swashbuckler Grace

def SwashbucklerGraceReflexBonus(attachee, args, evt_obj):
	#Must not be encumbered
	if SwashbucklerEncumberedCheck(attachee):
		return 0

	classLvl = attachee.stat_level_get(classEnum)
	classBonusLvls = attachee.d20_query("Swashbuckler Grace Level Bonus")
	classLvl = classLvl + classBonusLvls
	if classLvl < 11:
		bonval = 1
	elif classLvl < 20:
		bonval = 2
	else:
		bonval = 3
	evt_obj.bonus_list.add(bonval, 0, "Swashbuckler Grace" ) #Competence Bonus
	return 0

swashbucklerGrace = PythonModifier("Swashbuckler Grace", 2) #Spare, Spare
swashbucklerGrace.MapToFeat("Swashbuckler Grace")
swashbucklerGrace.AddHook(ET_OnSaveThrowLevel , EK_SAVE_REFLEX , SwashbucklerGraceReflexBonus, ())

# Swashbuckler Insightful Strike

def SwashbucklerInsightfulStrikeDamageBonus(attachee, args, evt_obj):
	#Must not be encumbered
	if SwashbucklerEncumberedCheck(attachee):
		return 0
	
	#Must be usable with weapon finesse
	weaponUsed = evt_obj.attack_packet.get_weapon_used()
	if not IsFinesseWeapon(attachee, weaponUsed):
		return 0
		
	#Enemy must be sneak attackable
	target = evt_obj.attack_packet.target
	if target.d20_query(Q_Critter_Is_Immune_Critical_Hits):
		return 0
	
	int = attachee.stat_level_get(stat_intelligence)
	intMod = (int - 10)/2
	evt_obj.damage_packet.bonus_list.add_from_feat(intMod, 0, 137, "Insightful Strike")
	return 0

swashbucklerInsightfulStrike = PythonModifier("Swashbuckler Insightful Strike", 2) #Spare, Spare
swashbucklerInsightfulStrike.MapToFeat("Swashbuckler Insightful Strike")
swashbucklerInsightfulStrike.AddHook(ET_OnDealingDamage, EK_NONE, SwashbucklerInsightfulStrikeDamageBonus, ())

# Swashbuckler Dodge

def SwashbucklerDodgeACBonus(attachee, args, evt_obj):
	#Must not be encumbered
	if SwashbucklerEncumberedCheck(attachee):
		return 0

	attacker = evt_obj.attack_packet.attacker
	if attacker == OBJ_HANDLE_NULL or attacker == attachee:
		return 0

	#Test if the ability is used

	prevAttacker = args.get_obj_from_args(0)

	#Works for each attack from the first attacker like dodge (SRD let you choose the opponent)
	if prevAttacker != OBJ_HANDLE_NULL:
		if attacker != prevAttacker:
			return 0
	
	classLvl = attachee.stat_level_get(classEnum)
	classBonusLvls = attachee.d20_query("Swashbuckler Dodge Level Bonus")
	classLvl = classLvl + classBonusLvls	
	bonval = classLvl / 5
	evt_obj.bonus_list.add(bonval, 8, 137 ) #Dodge bonus
	args.set_args_from_obj(0, attacker)
	return 0

def SwashbucklerDodgeBeginRound(attachee, args, evt_obj):
	#Reset to a null attacker at the beginning of the round
	args.set_args_from_obj(0, OBJ_HANDLE_NULL)
	return 0

swashbucklerDodge = PythonModifier("Swashbuckler Dodge", 4) #Used this round flag, Attacker Upper Handle,  Attacker Lower Handle, Spare
swashbucklerDodge.MapToFeat("Swashbuckler Dodge")
swashbucklerDodge.AddHook(ET_OnGetAC, EK_NONE, SwashbucklerDodgeACBonus, ())
swashbucklerDodge.AddHook(ET_OnBeginRound, EK_NONE, SwashbucklerDodgeBeginRound, ())
swashbucklerDodge.AddHook(ET_OnConditionAdd, EK_NONE, SwashbucklerDodgeBeginRound, ())

# Swashbuckler Acrobatic Charge

swashbucklerAcrobaticCharge = PythonModifier("Swashbuckler Acrobatic Charge", 2) #Used this round flag, Spare
swashbucklerAcrobaticCharge.MapToFeat("Swashbuckler Acrobatic Charge")

#Swashbuckler Improved Flanking

def SwashbucklerImprovedFlankingAttack(attachee, args, evt_obj):
	if evt_obj.attack_packet.get_flags() & D20CAF_FLANKED:
		evt_obj.bonus_list.add(2, 0, "Swashbuckler Improved Flanking")
	return 0

swashbucklerImprovedFlanking = PythonModifier("Swashbuckler Improved Flanking", 2) #Spare, Spare
swashbucklerImprovedFlanking.MapToFeat("Swashbuckler Improved Flanking")
swashbucklerImprovedFlanking.AddHook(ET_OnToHitBonus2, EK_NONE, SwashbucklerImprovedFlankingAttack, ())

# Swashbuckler Lucky

def SwashbucklerLuckyRerollSavingThrow(attachee, args, evt_obj):
	if args.get_arg(0) and args.get_arg(1):
		if not evt_obj.return_val:
			evt_obj.return_val = 1
			args.set_arg(0,0)
	return 0

def SwashbucklerLuckyRerollAttack(attachee, args, evt_obj):
	if args.get_arg(0) and args.get_arg(2):
		if not evt_obj.return_val:
			evt_obj.return_val = 1
			args.set_arg(0,0)
		
	return 0
	
def SwashbucklerLuckyRadial(attachee, args, evt_obj):
	#Add a checkbox to use the reroll if a charge is available
	if args.get_arg(0):
		radial_parent = tpdp.RadialMenuEntryParent("Lucky")
		LuckyID = radial_parent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
		checkboxSavingThrow = tpdp.RadialMenuEntryToggle("Reroll Next Missed Saving Throw", "TAG_INTERFACE_HELP")
		checkboxSavingThrow.link_to_args(args, 1)
		checkboxSavingThrow.add_as_child(attachee, LuckyID)
		checkboxAttack = tpdp.RadialMenuEntryToggle("Reroll Next Missed Attack", "TAG_INTERFACE_HELP")
		checkboxAttack.link_to_args(args, 2)
		checkboxAttack.add_as_child(attachee, LuckyID)
		
	return 0
	
def SwashbucklerLuckyNewDay(attachee, args, evt_obj):
	args.set_arg(0, 1)
	return 0

swashbucklerLucky = PythonModifier("Swashbuckler Lucky", 5) #Used, Reroll Saving Throw, Reroll Attack, Spare, Spare
swashbucklerLucky.MapToFeat("Swashbuckler Lucky")
swashbucklerLucky.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, SwashbucklerLuckyRadial, ())
swashbucklerLucky.AddHook(ET_OnD20Query, EK_Q_RerollSavingThrow, SwashbucklerLuckyRerollSavingThrow, ())
swashbucklerLucky.AddHook(ET_OnD20Query, EK_Q_RerollAttack, SwashbucklerLuckyRerollAttack, ())
swashbucklerLucky.AddHook(ET_OnConditionAdd, EK_NONE, SwashbucklerLuckyNewDay, ())
swashbucklerLucky.AddHook(ET_OnNewDay, EK_NEWDAY_REST, SwashbucklerLuckyNewDay, ())

# Swashbuckler Acrobatic Skill Mastery

swashbucklerAcrobaticSkillMastery = PythonModifier("Swashbuckler Acrobatic Skill Mastery", 2) #Spare, Spare
swashbucklerAcrobaticSkillMastery.MapToFeat("Swashbuckler Acrobatic Skill Mastery")

# Swashbuckler Weakening Critical
def SwashbucklerWeakeningCriticalOnDamage(attachee, args, evt_obj):
	#Enemy must not be immune to criticals
	target = evt_obj.attack_packet.target
	if target.d20_query(Q_Critter_Is_Immune_Critical_Hits):
		return 0
	
	attackFlags = evt_obj.attack_packet.get_flags()

	#Must be a critical
	criticalHit = attackFlags & D20CAF_CRITICAL
	if not criticalHit:
		return 0
	
	target.condition_add_with_args( "Damage_Ability_Loss", 0, 2)

	game.create_history_freeform(target.description + " takes 2 points of strength damage from weakening critical.\n\n")
	target.float_text_line("Strength damage!")
	return 0

swashbucklerWeakeningCritical = PythonModifier("Swashbuckler Weakening Critical", 2) #Spare, Spare
swashbucklerWeakeningCritical.MapToFeat("Swashbuckler Weakening Critical")
swashbucklerWeakeningCritical.AddHook(ET_OnDealingDamage2, EK_NONE, SwashbucklerWeakeningCriticalOnDamage, ())

# Swashbuckler Wounding Critical

def SwashbucklerWoundingCriticalOnDamage(attachee, args, evt_obj):
	#Enemy must not be immune to criticals
	target = evt_obj.attack_packet.target
	if target.d20_query(Q_Critter_Is_Immune_Critical_Hits):
		return 0
	
	attackFlags = evt_obj.attack_packet.get_flags()
	
	#Must be a critical
	criticalHit = attackFlags & D20CAF_CRITICAL
	if not criticalHit:
		return 0
	
	target.condition_add_with_args( "Damage_Ability_Loss", 2, 2)

	game.create_history_freeform(target.description + " takes 2 points of constitution damage from wounding critical.\n\n")
	target.float_text_line("Constitution damage!")
	return 0

swashbucklerWoundingCritical = PythonModifier("Swashbuckler Wounding Critical", 2) #Spare, Spare
swashbucklerWoundingCritical.MapToFeat("Swashbuckler Wounding Critical")
swashbucklerWeakeningCritical.AddHook(ET_OnDealingDamage2, EK_NONE, SwashbucklerWoundingCriticalOnDamage, ())
