from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

###################################################

def GetConditionName():
	return "Arcane Trickster"

def GetSpellCasterConditionName():
	return "Arcane Trickster Spellcasting"
	
print "Registering " + GetConditionName()

classEnum = stat_level_arcane_trickster
classSpecModule = __import__('class019_arcane_trickster')
impromptuSneakEnum = 1900
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


def ArcTrkSneakAttackDice(attachee, args, evt_obj):
	arcTrkLvl = attachee.stat_level_get(classEnum)
	if evt_obj.data1 == classEnum: #class leveling up
		arcTrkLvl = arcTrkLvl + 1
	if arcTrkLvl <= 0:
		return 0
	evt_obj.return_val += arcTrkLvl /2
	return 0


def ImpromptuSneakAttack(attachee, args, evt_obj):
	attachee.condition_add_with_args("Impromptu Sneak Attack",0,0)
	arcTrkLvl = attachee.stat_level_get(classEnum)
	if arcTrkLvl - 3 < 0:
		return 0
	radialAction = tpdp.RadialMenuEntryPythonAction(-1, D20A_PYTHON_ACTION, impromptuSneakEnum, 0,
													"TAG_INTERFACE_HELP")
	radialAction.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
	return 0


classSpecObj = PythonModifier(GetConditionName(), 0)
classSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())
classSpecObj.AddHook(ET_OnD20PythonQuery, "Sneak Attack Dice", ArcTrkSneakAttackDice, ())
classSpecObj.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, ImpromptuSneakAttack, ())

##### Spell casting

# configure the spell casting condition to hold the highest Arcane classs
def OnAddSpellCasting(attachee, args, evt_obj):
	# arg0 holds the arcane class
	if args.get_arg(0) == 0:
		args.set_arg(0, char_class_utils.GetHighestArcaneClass(attachee))

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

# Impromptu Sneak Attack

def IsActive(args):
	if args.get_arg(0):
		return 1
	return 0

def ImpSneakDamageIsActive(attachee, args, evt_obj):
	if args.get_arg(2):
		evt_obj.return_val = 1
		return 0
	return 0

def ImpSneakDamagedApplied(attachee, args, evt_obj):
	args.set_arg(2, 0) # unset
	return 0

def ImpSneakAttackRollMade(attachee, args, evt_obj):
	if IsActive(args):
		args.set_arg(0, 0)
		args.set_arg(2, 1)
	return 0


def ImpSneakDexterityNullifier(attachee, args, evt_obj):
	if not IsActive(args):
		return 0
	evt_obj.bonus_list.add_cap(3, 0, 202)
	return 0


def ImpSneakNewday(attachee, args, evt_obj):
	args.set_arg(0, 0)
	args.set_arg(1, 0)
	args.set_arg(2, 0)
	return 0

def OnImpSneakCheck(attachee, args, evt_obj):
	if IsActive(args):
		evt_obj.return_val = AEC_INVALID_ACTION
		return 0

	# check if enough usages / day left
	arcTrkLvl = attachee.stat_level_get(classEnum)
	maxNumPerDay = 1 + (arcTrkLvl-3)/4
	if args.get_arg(1) >= maxNumPerDay:
		evt_obj.return_val = AEC_OUT_OF_CHARGES
	return 0

def OnImpSneakPerform(attachee, args, evt_obj):
	if IsActive(args):
		return 0
	args.set_arg(0, 1) # set to active
	args.set_arg(1, args.get_arg(1) + 1) #increment number used / day
	args.set_arg(2, 0)  # reset expecting damage state
	attachee.float_text_line("Sneak Attacking", tf_red)
	return 0

impSneak = PythonModifier("Impromptu Sneak Attack", 3) # arg0 - is active; arg1 - times spent; arg2 - anticipate damage roll with sneak attack
impSneak.AddHook(ET_OnD20Query, EK_Q_OpponentSneakAttack, ImpSneakDamageIsActive, ())
impSneak.AddHook(ET_OnD20Signal, EK_S_Attack_Made, ImpSneakDamagedApplied, ()) # gets triggered at the end of the damage calculation
impSneak.AddHook(ET_OnGetAcModifierFromAttacker, EK_NONE, ImpSneakAttackRollMade, ()) # signifies that a to hit roll was made
impSneak.AddHook(ET_OnGetAcModifierFromAttacker, EK_NONE, ImpSneakDexterityNullifier, () )
impSneak.AddHook(ET_OnD20PythonActionCheck, impromptuSneakEnum, OnImpSneakCheck, ())
impSneak.AddHook(ET_OnD20PythonActionPerform, impromptuSneakEnum, OnImpSneakPerform, ())
impSneak.AddHook(ET_OnNewDay, EK_NEWDAY_REST, ImpSneakNewday, ())