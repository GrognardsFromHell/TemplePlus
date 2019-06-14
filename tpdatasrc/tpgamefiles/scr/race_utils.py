from __main__ import game
from toee import *
import race_defs

#########################################
# Ability scores
#########################################
def OnGetAbilityScore(attachee, args, evt_obj):
	statType = args.get_param(0)
	statMod = args.get_param(1)
	if attachee.d20_query(Q_Polymorphed):
		if (statType == stat_strength) or (statType == stat_constitution) or (statType == stat_dexterity):
			return 0
	newValue = statMod + evt_obj.bonus_list.get_sum()
	if (newValue < 3): # ensure minimum stat of 3
		statMod = 3-newValue
	evt_obj.bonus_list.add(statMod, 0, 139)
	return 0

def AddAbilityModifierHooks(raceSpecObj, raceSpecModule):
	statMods = race_defs.GetStatModifiers(raceSpecModule.raceEnum) #raceSpecModule.GetStatModifiers()
	for p in range(0,6):
		if statMods[p] == 0:
			continue
		statType = p
		statMod = statMods[p]
		raceSpecObj.AddHook(ET_OnAbilityScoreLevel, EK_STAT_STRENGTH+p, OnGetAbilityScore, (p,statMods[p]))
		raceSpecObj.AddHook(ET_OnStatBaseGet, EK_STAT_STRENGTH+p, OnGetAbilityScore, (p,statMods[p]))
	return



#########################################
## Saving Throws
#########################################
def OnGetSaveThrow(attachee, args, evt_obj):
	value = args.get_param(0)
	evt_obj.bonus_list.add(value, 0, 139)
	return 0

def AddSaveThrowBonusHook(raceSpecObj, saveThrowType, value):
	k = EK_SAVE_FORTITUDE + saveThrowType
	raceSpecObj.AddHook(ET_OnSaveThrowLevel, k, OnGetSaveThrow, (value,))
	return

def OnGetSaveThrow(attachee, args, evt_obj):
	saveThrowDescriptor = args.get_param(0) # D20STD_xxx
	effectTypeFlags     = 1<<(saveThrowDescriptor-1)
	value = args.get_param(1)
	if evt_obj.flags & effectTypeFlags:
		evt_obj.bonus_list.add(value, 0, 139)
	return 0

def AddSaveBonusVsEffectType(raceSpecObj, saveThrowDescriptor, value):
	raceSpecObj.AddHook(ET_OnSaveThrowLevel, EK_NONE, OnGetSaveThrow, (saveThrowDescriptor,value))
	return
#########################################


#########################################
## Immunities and Resistances          ##
#########################################
def OnQueryReturnTrue(attachee, args, evt_obj):
	evt_obj.return_val = 1
	return 0
def ConditionImmunityOnPreAdd(attachee, args, evt_obj):
	val = evt_obj.is_modifier("Poisoned")
	if val:
		evt_obj.return_val = 0
		attachee.float_text_line( "Poison Immunity", tf_red )
	return 0
def AddPoisonImmunity(raceSpecObj):
	raceSpecObj.AddHook(ET_OnD20Query       , EK_Q_Critter_Is_Immune_Poison, OnQueryReturnTrue, ())
	raceSpecObj.AddHook(ET_OnConditionAddPre, EK_NONE                      , ConditionImmunityOnPreAdd, ())
	return

def OnGetDamageResistance(attachee, args, evt_obj):
	damType = args.get_param(0)
	value = args.get_param(1)
	DAMAGE_MES_RESISTANCE_TO_ENERGY = 124
	evt_obj.damage_packet.add_damage_resistance(value,damType, DAMAGE_MES_RESISTANCE_TO_ENERGY)
	return 0
def AddDamageResistances(raceSpecObj, damResistanceDict):
	for k,v in damResistanceDict.items():
		raceSpecObj.AddHook(ET_OnTakingDamage, EK_NONE, OnGetDamageResistance, (k,v))
	return




#########################################
## Skill Bonuses
#########################################
def OnGetSkillLevel(attachee, args, evt_obj):
	value = args.get_param(0)
	evt_obj.bonus_list.add(value, 0, 139)
	return 0

def AddSkillBonuses(raceSpecObj, skillBonusDict):
	for k,v in skillBonusDict.items():
		#print "Skill bonus: " + str(k) + " " + str(v)
		raceSpecObj.AddHook(ET_OnGetSkillLevel, EK_SKILL_APPRAISE + k, OnGetSkillLevel, (v,))
	return
#########################################


#########################################
## favored class
#########################################
def OnGetFavoredClass(attachee, args, evt_obj):
	classEnum = args.get_param(0)
	if evt_obj.data1 == classEnum:
		evt_obj.return_val = 1
	return 0

def AddFavoredClassHook(raceSpecObj, classEnum):
	raceSpecObj.AddHook(ET_OnD20Query, EK_Q_FavoredClass, OnGetFavoredClass, (classEnum,))
#########################################


#########################################
## Base move speed
#########################################
def OnGetBaseMoveSpeed(attachee, args, evt_obj):
	val = args.get_param(0)
	evt_obj.bonus_list.add( val, 1, 139)
	return 0

def AddBaseMoveSpeed(raceSpecObj, value):
	raceSpecObj.AddHook(ET_OnGetMoveSpeedBase, EK_NONE, OnGetBaseMoveSpeed, (value,))
