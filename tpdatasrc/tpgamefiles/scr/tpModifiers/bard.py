from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

###################################################

def GetConditionName():
	return "Bard"
	
def GetSpellCasterConditionName():
	return "Bard Spellcasting"

print "Registering " + GetSpellCasterConditionName()

classEnum = stat_level_bard
classSpecModule = __import__('class008_bard')
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
	#print "Bard OnGetBaseCasterLevel: Called with arg0 " + str(evt_obj.arg0)
	if evt_obj.arg0 != classEnum:
		return 0
	classLvl = attachee.stat_level_get(classEnum)
	evt_obj.bonus_list.add(classLvl, 0, 137)
	return 0

def OnInitLevelupSpellSelection(attachee, args, evt_obj):
	#print "Bard OnInitLevelupSpellSelection: Called with arg0 " + str(evt_obj.arg0)
	if evt_obj.arg0 != classEnum:
		return 0
	classSpecModule.InitSpellSelection(attachee)
	return 0

def OnLevelupSpellsCheckComplete(attachee, args, evt_obj):
	#print "Bard OnLevelupSpellsCheckComplete: Called with arg0 " + str(evt_obj.arg0)
	if evt_obj.arg0 != classEnum:
		return 0
	if not classSpecModule.LevelupCheckSpells(attachee):
		evt_obj.bonus_list.add(-1, 0, 137) # denotes incomplete spell selection
	return 1
	
def OnLevelupSpellsFinalize(attachee, args, evt_obj):
	#print "Bard OnLevelupSpellsFinalize: Called with arg0 " + str(evt_obj.arg0)
	if evt_obj.arg0 != classEnum:
		return 0
	classSpecModule.LevelupSpellsFinalize(attachee)
	return
# spellCasterSpecObj = PythonModifier(GetSpellCasterConditionName(), 8)
# spellCasterSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())

def BardSpellFailure(attachee, args, evt_obj):
	if evt_obj.data1 != classEnum:
		return 0

	equip_slot = evt_obj.data2
	item = attachee.item_worn_at(equip_slot)

	if item == OBJ_HANDLE_NULL:
		return 0

	if equip_slot == 5: # armor - bards can cast in light armor with no spell failure
		armor_flags = item.obj_get_int(obj_f_armor_flags)
		if (armor_flags & ARMOR_TYPE_NONE) or (armor_flags == ARMOR_TYPE_LIGHT):
			return 0

	evt_obj.return_val += item.obj_get_int(obj_f_armor_arcane_spell_failure)
	return 0

classSpecExtender = PythonModifier()
classSpecExtender.ExtendExisting("Bard")
classSpecExtender.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
classSpecExtender.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Activate, OnInitLevelupSpellSelection, ())
classSpecExtender.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())
classSpecExtender.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Check_Complete, OnLevelupSpellsCheckComplete, ())
classSpecExtender.AddHook(ET_OnD20Query, EK_Q_Get_Arcane_Spell_Failure, BardSpellFailure, ())




bardFascEnum = 801

def GetBardicMusicType(attachee, args, evt_obj):
	evt_obj.return_val = args.get_arg(1)
	return 0

def BardicMusicEnd(attachee, args, evt_obj):
	partsysId = args.get_arg(5)
	if partsysId:
		game.particles_kill(partsysId)
		args.set_arg(5,0)
	args.set_arg(1,0) # music type currently playing
	args.set_arg(2,0) # rounds music played
	args.set_arg(3,0) # target handle
	args.set_arg(4,0) # target handle upper
	return 0

bardicMus = PythonModifier()
bardicMus.ExtendExisting("Bardic Music")
bardicMus.AddHook(ET_OnD20PythonQuery, "Bardic Music Type", GetBardicMusicType, ())
bardicMus.AddHook(ET_OnD20PythonSignal, "Bardic Music End", BardicMusicEnd, ())
#bardicMus.AddHook(ET_OnD20PythonActionPerform, bardFascEnum, OnStudyTargetPerform, ())