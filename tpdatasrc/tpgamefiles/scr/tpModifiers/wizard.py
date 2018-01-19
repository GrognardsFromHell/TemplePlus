from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils


###################################################

def GetConditionName():
	return "Wizard"
	
def GetSpellCasterConditionName():
	return "Wizard Spellcasting"

print "Registering " + GetSpellCasterConditionName()

classEnum = stat_level_wizard
classSpecModule = __import__('class017_wizard')

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


#classSpecObj = PythonModifier(GetConditionName(), 0)
#classSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
#classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
#classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
#classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())


### Spell casting
def OnGetBaseCasterLevel(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	classLvl = attachee.stat_level_get(classEnum)
	evt_obj.bonus_list.add(classLvl, 0, 137)
	return 0

# spellCasterSpecObj = PythonModifier(GetSpellCasterConditionName(), 8)
# spellCasterSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())

def OnLevelupSpellsCheckComplete(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	if not classSpecModule.LevelupCheckSpells(attachee):
		evt_obj.bonus_list.add(-1, 0, 137) # denotes incomplete spell selection
	return 1

def OnInitLevelupSpellSelection(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	classSpecModule.InitSpellSelection(attachee)
	return 0

def OnLevelupSpellsFinalize(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	classSpecModule.LevelupSpellsFinalize(attachee)
	return

def ArcaneSpellFailure(attachee, args, evt_obj):
	if evt_obj.data1 != classEnum and evt_obj.data1 != stat_level_sorcerer:
		return 0

	equip_slot = evt_obj.data2
	item = attachee.item_worn_at(equip_slot)

	if item == OBJ_HANDLE_NULL:
		return 0

	evt_obj.return_val += item.obj_get_int(obj_f_armor_arcane_spell_failure)
	return 0

classSpecExtender = PythonModifier()
classSpecExtender.ExtendExisting("Wizard")
classSpecExtender.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
classSpecExtender.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Activate, OnInitLevelupSpellSelection, ())
classSpecExtender.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())
classSpecExtender.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Check_Complete, OnLevelupSpellsCheckComplete, ())
classSpecExtender.AddHook(ET_OnD20Query, EK_Q_Get_Arcane_Spell_Failure, ArcaneSpellFailure, ())
