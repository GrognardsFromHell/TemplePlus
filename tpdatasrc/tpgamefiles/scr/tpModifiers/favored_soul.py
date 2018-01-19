from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

###################################################

def GetConditionName():
	return "Favored Soul"
	
# def GetSpellCasterConditionName():
	# return "Sorcerer Spellcasting"

# print "Registering " + GetSpellCasterConditionName()

classEnum = stat_level_favored_soul
classSpecModule = __import__('class034_favored_soul')
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

def OnLevelupSpellsFinalize(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	classSpecModule.LevelupSpellsFinalize(attachee)
	return

# spellCasterSpecObj = PythonModifier(GetSpellCasterConditionName(), 8)
# spellCasterSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())


classSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
classSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Activate, OnInitLevelupSpellSelection, ())
classSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())
classSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Check_Complete, OnLevelupSpellsCheckComplete, ())


## Feats

# Energy Resistance

def FavSoulEnergyRes(attachee, args, evt_obj):
	#print "Fav Soul Energy Resistance"
	damType = args.get_param(0)
	#print str(damType)
	evt_obj.damage_packet.add_damage_resistance(10,damType, 124)
	return 0

favSoulEnergyResAcid = PythonModifier('Favored Soul Acid Resistance', 3)
favSoulEnergyResAcid.MapToFeat('Favored Soul Acid Resistance')
favSoulEnergyResAcid.AddHook(ET_OnTakingDamage, EK_NONE, FavSoulEnergyRes, (D20DT_ACID,))

favSoulEnergyResCold = PythonModifier('Favored Soul Cold Resistance', 3)
favSoulEnergyResCold.MapToFeat('Favored Soul Cold Resistance')
favSoulEnergyResCold.AddHook(ET_OnTakingDamage, EK_NONE, FavSoulEnergyRes, (D20DT_COLD,))

favSoulEnergyResElectr = PythonModifier('Favored Soul Electricity Resistance', 3)
favSoulEnergyResElectr.MapToFeat('Favored Soul Electricity Resistance')
favSoulEnergyResElectr.AddHook(ET_OnTakingDamage, EK_NONE, FavSoulEnergyRes, (D20DT_ELECTRICITY,)  )

favSoulEnergyResFire = PythonModifier('Favored Soul Fire Resistance', 3)
favSoulEnergyResFire.MapToFeat('Favored Soul Fire Resistance')
favSoulEnergyResFire.AddHook(ET_OnTakingDamage, EK_NONE, FavSoulEnergyRes, (D20DT_FIRE,))

favSoulEnergyResSonic = PythonModifier('Favored Soul Sonic Resistance', 3)
favSoulEnergyResSonic.MapToFeat('Favored Soul Sonic Resistance')
favSoulEnergyResSonic.AddHook(ET_OnTakingDamage, EK_NONE, FavSoulEnergyRes, (D20DT_SONIC,))



# Damage reduction

def FavoredSoulDR(attachee, args, evt_obj):
	fav_soul_lvl = attachee.stat_level_get(classEnum)
	if fav_soul_lvl < 20:
		return 0
	bonval = 10
	align = attachee.stat_level_get(stat_alignment)
	if align & ALIGNMENT_CHAOTIC:
		evt_obj.damage_packet.add_physical_damage_res(bonval, D20DAP_COLD, 126)  # DR 10/Cold Iron
	else:
		evt_obj.damage_packet.add_physical_damage_res(bonval, D20DAP_SILVER, 126)  # DR 10/Silver
		#skipped implementing the choice for neutrals, partly because there's no Cold Iron in ToEE (and Silver is pretty rare too)
	return 0

classSpecObj.AddHook(ET_OnTakingDamage2, EK_NONE, FavoredSoulDR, ())