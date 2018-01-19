from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils
import d20_action_utils
###################################################

def GetConditionName():
	return "Dwarven Defender"

print "Registering " + GetConditionName()

classEnum = stat_level_dwarven_defender
defensiveStanceEnum = 2500
defensiveStanceWindedEnum = 2501

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


def DwarvenDefenderAcBonus(attachee, args, evt_obj):
	dwdLvl = attachee.stat_level_get(classEnum)
	bonval = 1 + (dwdLvl-1)/3

	evt_obj.bonus_list.add(bonval, 8, 137)  # Dodge bonus,  ~Class~[TAG_LEVEL_BONUSES]
	return 0

def DwDTrapSenseDodgeBonus(attachee, args, evt_obj):
	dwdLvl = attachee.stat_level_get(classEnum)
	if dwdLvl < 4:
		return 0
	bonval = 1+(dwdLvl-4)/4
	if evt_obj.attack_packet.get_flags() & D20CAF_TRAP:
		evt_obj.bonus_list.add(bonval, 8, 137 )
	return 0

def DwDTrapSenseReflexBonus(attachee, args, evt_obj):
	dwdLvl = attachee.stat_level_get(classEnum)
	if dwdLvl < 4:
		return 0
	bonval = 1+(dwdLvl-4)/4
	if evt_obj.flags & 2:
		evt_obj.bonus_list.add(bonval, 8, 137 )
	return 0

def DwDDamageReduction(attachee, args, evt_obj):
	dwdLvl = attachee.stat_level_get(classEnum)
	if dwdLvl < 6:
		return 0
	bonval = 3*(1+(dwdLvl-6)/4)
	evt_obj.damage_packet.add_physical_damage_res(bonval, 1, 126 ) # type 1 - will always apply
	return 0


def DefensiveStanceRadial(attachee, args, evt_obj):
	isAdded = attachee.condition_add_with_args("Defensive Stance",0,0) # adds the "Defensive Stance" condition on first radial menu build
	isWinded = 0
	isActive = 0
	# add radial menu action Defensive Stanch
	if not isAdded: # means it's not a newly added condition
		isWinded = attachee.d20_query("Defensive Stance Is Winded")
		isActive = attachee.d20_query("Defensive Stance Is Active")
	#print "is active: " + str(isActive) + "  is winded: " + str(isWinded)
	if isActive and (isWinded == 0): # if already active, show the "Winded" option a la Barbarian Rage
		radialAction = tpdp.RadialMenuEntryPythonAction(-1, D20A_PYTHON_ACTION, defensiveStanceWindedEnum, 0, "TAG_INTERFACE_HELP")
	else:
		#print str(D20A_PYTHON_ACTION) + "  " + str(defensiveStanceEnum)
		radialAction = tpdp.RadialMenuEntryPythonAction(-1, D20A_PYTHON_ACTION, defensiveStanceEnum, 0, "TAG_INTERFACE_HELP")
	radialAction.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
	return 0



classSpecObj = PythonModifier(GetConditionName(), 0)
classSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())
classSpecObj.AddHook(ET_OnGetAC, EK_NONE, DwarvenDefenderAcBonus, ())
classSpecObj.AddHook(ET_OnGetAC, EK_NONE, DwDTrapSenseDodgeBonus, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel , EK_SAVE_REFLEX , DwDTrapSenseReflexBonus, ())
classSpecObj.AddHook(ET_OnTakingDamage2, EK_NONE, DwDDamageReduction, ())
classSpecObj.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, DefensiveStanceRadial, ())

def DefensiveStanceIsWinded(attachee, args, evt_obj):
	if args.get_arg(3) != 0:
		evt_obj.return_val = 1
	return 0


def IsActive(args):
	isActive = args.get_arg(0)
	if isActive:
		return 1
	return 0

def IsWinded(args):
	if not IsActive(args):
		return 0
	isWinded = args.get_arg(3)
	if isWinded:
		return 1
	return 0

def DefensiveStanceIsActive(attachee, args, evt_obj):
	if IsActive(args):
		evt_obj.return_val = 1
	return 0

def OnDefensiveStanceCheck(attachee, args, evt_obj):
	if IsActive(args): # if already active
		if IsWinded(args):
			evt_obj.return_val = AEC_INVALID_ACTION
			return 0
		else: # action is possible (will make attachee winded)
			return 0

	dwdLvl = attachee.stat_level_get(classEnum)
	maxNumPerDay = 1 + (dwdLvl-1)/2
	if args.get_arg(1) >= maxNumPerDay:
		evt_obj.return_val = AEC_OUT_OF_CHARGES
	return 0


def OnDefensiveStancePerform(attachee, args, evt_obj):
	print "Performing def stance"
	if IsActive(args): # change to winded
		print "changing to winded"
		args.set_arg(3, 1)
		args.set_arg(2, 10) # for 10 rounds
		return 0

	args.set_arg(0, 1) # set to active
	args.set_arg(1, args.get_arg(1) + 1) # increase number of times used today
	conLvl = attachee.stat_level_get(stat_constitution)
	conLvl += 4 # constitution is increased by 4 and this counts towards the number of rounds
	conMod = (conLvl - 10)/2
	numRounds = max(1,3 + conMod)
	args.set_arg(2, numRounds ) # set the number of rounds remaining
	args.set_arg(3, 0) # set isWinded to 0
	return 0



def DefStanceConMod(attachee, args, evt_obj):
	if not IsActive(args):
		return 0
	if IsWinded(args):
		return 0
	evt_obj.bonus_list.add(4, 0, 137)
	return 0

def DefStanceStrMod(attachee, args, evt_obj):
	if not IsActive(args):
		return 0
	if IsWinded(args):
		evt_obj.bonus_list.add(-2, 0, 137)
		return 0
	evt_obj.bonus_list.add(2, 0, 137)
	return 0

def DefStanceMoveSpeed(attachee, args, evt_obj):
	if not IsActive(args):
		return 0
	if IsWinded(args):
		return 0
	movespeedCap = 0
	if attachee.stat_level_get(classEnum) >= 8:
		movespeedCap = 5
	evt_obj.bonus_list.set_overall_cap(1, movespeedCap, 0, 137) # set upper cap
	evt_obj.bonus_list.set_overall_cap(6, 0, 0, 137) # lower cap... set with the override flag (4) because dwarves can have a lower racial cap of 20
	return 0

def DefStanceSaveBonus(attachee, args, evt_obj):
	if not IsActive(args):
		return 0
	if IsWinded(args):
		return 0
	evt_obj.bonus_list.add(2, 0, 137)
	return 0

def DefStanceAcBonus(attachee, args, evt_obj):
	if not IsActive(args):
		return 0
	if IsWinded(args):
		return 0
	evt_obj.bonus_list.add(4, 8, 137)
	return 0

def DefStanceBeginRound(attachee, args, evt_obj):
	if not IsActive(args): # not active
		return 0
	numRounds = args.get_arg(2)
	roundsToReduce = evt_obj.data1
	if numRounds - roundsToReduce >= 0:
		#print "beginround: new rounds remaining: " + str(numRounds - roundsToReduce )
		args.set_arg(2, numRounds - roundsToReduce )
		return 0
	else:
		if not IsWinded(args):
			#print "beginround: setting to winded"
			args.set_arg(3, 1) # set winded
			args.set_arg(2, 10) # setting winded to 10 rounds instead of the nebulous "duration of the encounter"
		else:
			#print "beginround: finishing winded"
			args.set_arg(3, 0) # unset winded
			args.set_arg(0, 0) # set inactive
			args.set_arg(2, 0) # reset remaining rounds
	return 0

def DefStanceNewday(attachee, args, evt_obj):
	args.set_arg(0, 0)
	args.set_arg(1, 0)
	args.set_arg(2, 0)
	args.set_arg(3, 0)
	return 0

def DefStanceTooltip(attachee, args, evt_obj):
	if not IsActive(args):
		return 0
	if IsWinded(args):
		evt_obj.append("Winded (" + str(args.get_arg(2)) + " rounds)")
		return 0
	evt_obj.append("Defensive Stance (" + str(args.get_arg(2)) + " rounds)")
	return 0

def DefStanceEffectTooltip(attachee, args, evt_obj):
	if not IsActive(args):
		return 0
	if IsWinded(args):
		evt_obj.append(154, -2, "Winded (" + str(args.get_arg(2)) + " rounds)")
		return 0
	evt_obj.append(53, -2, "Defensive Stance (" + str(args.get_arg(2)) + " rounds)")
	return 0

defStance = PythonModifier("Defensive Stance", 4) # arg0 - is active ; arg1 - number of times used this day ; arg2 - rounds remaining ; arg3 - is in winded state
defStance.AddHook(ET_OnD20PythonQuery, "Defensive Stance Is Winded", DefensiveStanceIsWinded, ())
defStance.AddHook(ET_OnD20PythonQuery, "Defensive Stance Is Active", DefensiveStanceIsActive, ())
defStance.AddHook(ET_OnD20PythonActionCheck, defensiveStanceEnum, OnDefensiveStanceCheck, ())
defStance.AddHook(ET_OnD20PythonActionPerform, defensiveStanceEnum, OnDefensiveStancePerform, ())
defStance.AddHook(ET_OnD20PythonActionCheck, defensiveStanceWindedEnum, OnDefensiveStanceCheck, ())
defStance.AddHook(ET_OnD20PythonActionPerform, defensiveStanceWindedEnum, OnDefensiveStancePerform, ())
defStance.AddHook(ET_OnAbilityScoreLevel, EK_STAT_CONSTITUTION, DefStanceConMod, ())
defStance.AddHook(ET_OnAbilityScoreLevel, EK_STAT_STRENGTH, DefStanceStrMod, ())
defStance.AddHook(ET_OnGetMoveSpeed, EK_NONE, DefStanceMoveSpeed, ())
defStance.AddHook(ET_OnSaveThrowLevel, EK_NONE, DefStanceSaveBonus, ())
defStance.AddHook(ET_OnGetAC, EK_NONE, DefStanceAcBonus, ())
defStance.AddHook(ET_OnBeginRound, EK_NONE, DefStanceBeginRound, ())
defStance.AddHook(ET_OnNewDay, EK_NEWDAY_REST, DefStanceNewday, ())
defStance.AddHook(ET_OnGetTooltip, EK_NONE, DefStanceTooltip, ())
defStance.AddHook(ET_OnGetEffectTooltip, EK_NONE, DefStanceEffectTooltip, ())