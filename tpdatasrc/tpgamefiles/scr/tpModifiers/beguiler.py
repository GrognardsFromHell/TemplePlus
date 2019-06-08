from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

###################################################

def GetConditionName():
	return "Beguiler"

print "Registering " + GetConditionName()

classEnum = stat_level_beguiler
classSpecModule = __import__('class048_beguiler')
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


### Spell casting
def OnGetBaseCasterLevel(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	classLvl = attachee.stat_level_get(classEnum)
	evt_obj.bonus_list.add(classLvl, 0, 137)
	return 0

def OnLevelupSpellsFinalize(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	classSpecModule.LevelupSpellsFinalize(attachee)
	return 0
	
def OnInitLevelupSpellSelection(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	classSpecModule.InitSpellSelection(attachee)
	return 0
	
def OnLevelupSpellsCheckComplete(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	if not classSpecModule.LevelupCheckSpells(attachee):
		evt_obj.bonus_list.add(-1, 0, 137) # denotes incomplete spell selection
	return 1

classSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
classSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())
classSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Activate, OnInitLevelupSpellSelection, ())
classSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Check_Complete, OnLevelupSpellsCheckComplete, ())


#Armored Mage Beguiler

def BeguilerSpellFailure(attachee, args, evt_obj):
	#Only effects spells cast as a beguiler
	if evt_obj.data1 != classEnum:
		return 0

	equip_slot = evt_obj.data2
	item = attachee.item_worn_at(equip_slot)

	if item == OBJ_HANDLE_NULL:
		return 0
		
	if equip_slot == item_wear_armor: # beguiler can cast in light armor (and medium armor at level 8 or greater) with no spell failure
		armor_flags = item.obj_get_int(obj_f_armor_flags)
		if (armor_flags & ARMOR_TYPE_NONE) or (armor_flags == ARMOR_TYPE_LIGHT):
			return 0
			
	evt_obj.return_val += item.obj_get_int(obj_f_armor_arcane_spell_failure)
	return 0

armoredMage = PythonModifier("Beguiler Armored Mage", 2) #Spare, Spare
armoredMage.MapToFeat("Beguiler Armored Mage")
armoredMage.AddHook(ET_OnD20Query, EK_Q_Get_Arcane_Spell_Failure, BeguilerSpellFailure, ())

#Cloaked Casting

def QuickenFeintCostMod(attachee, args, evt_obj):
	if evt_obj.d20a.action_type != tpdp.D20ActionType.Feint:
		return 0
		
	#Change to a swift or move equivalent action as appropriate
	if attachee.has_feat(feat_improved_feint):
		if not (evt_obj.turnbased_status.flags & TBSF_FreeActionSpellPerformed):
			evt_obj.turnbased_status.flags |= TBSF_FreeActionSpellPerformed #Swift action uses the quickened spell action
			evt_obj.cost_new.action_cost = D20ACT_NULL
	else:
		evt_obj.cost_new.action_cost = D20ACT_Move_Action
	return 0

surpriseCasting = PythonModifier("Surprise Casting", 2) #Spare, Spare
surpriseCasting.MapToFeat("Surprise Casting")
surpriseCasting.AddHook(ET_OnActionCostMod, EK_NONE, QuickenFeintCostMod, ())

def CloakedCastingDCMod(attachee, args, evt_obj):
	# Check for flat footed or can't sense
	if evt_obj.target.can_sense(attachee) and not evt_obj.target.d20_query(Q_Flatfooted):
		return 0

	classLvl = attachee.stat_level_get(classEnum)
	if classLvl > 13:
		evt_obj.bonus_list.add(2, 0, "Cloaked Casting")
	elif classLvl > 1:
		evt_obj.bonus_list.add(1, 0, "Cloaked Casting")	
	
	return 0

def CloakedCastingResistanceMod(attachee, args, evt_obj):
	# Check for flat footed or can't sense
	if evt_obj.target.can_sense(attachee) and not evt_obj.target.d20_query(Q_Flatfooted):
		return 0

	classLvl = attachee.stat_level_get(classEnum)
	if classLvl > 19:
		evt_obj.bonus_list.add(1000, 0, "Cloaked Casting") #Arbitrary value so the check never fails
	elif classLvl > 7:
		evt_obj.bonus_list.add(2, 0, "Cloaked Casting")	
	
	return 0
	
cloakedCasting = PythonModifier("Cloaked Casting", 2) #Spare, Spare
cloakedCasting.MapToFeat("Cloaked Casting")
cloakedCasting.AddHook(ET_OnTargetSpellDCBonus, EK_NONE, CloakedCastingDCMod, ())
cloakedCasting.AddHook(ET_OnSpellResistanceCheckBonus, EK_NONE, CloakedCastingResistanceMod, ())



