from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

###################################################

def GetConditionName():
	return "Warmage"
	
print "Registering " + GetConditionName()

classEnum = stat_level_warmage
classSpecModule = __import__('class047_warmage')
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

classSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
classSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())

# Warmage Edge
# Here is how this complicated ability is implimented.  First it is increated by a critical but not empower
# Second multi target spells will only get the benefit once.  This will effect the first one to do damage.
# Third area area effect spells get the benefit against each object in their area of effect one time (ice
# storm for example only gets the bonus on the bludgeoning damage).  Fourth multi round spells can get the 
# benefit each round.

# List of who has been effected by the the last spell that caused damage.  Not a prefect solution but this should
# work fairly well.
effectedList = list()

def WarmageBeginRound(attachee, args, evt_obj):
	args.set_arg(0, 0)
	effectedList[:] = []
	return 0

def WarmageEdgeOnSpellDamage(attachee, args, evt_obj):
	
	#Only effects warmage spells
	spellCastingClass = evt_obj.spell_packet.get_spell_casting_class()
	if spellCastingClass != stat_level_warmage:
		return 0
	
	#Clear the effected list if this is a new spell
	prevSpellID = args.get_arg(0)
	spellID = evt_obj.spell_packet.spell_id
	
	if prevSpellID != spellID:
		effectedList[:] = []
	
	spEntry = tpdp.SpellEntry(evt_obj.spell_packet.spell_enum)
	multiTarget = spEntry.is_base_mode_target(MODE_TARGET_MULTI)  	
	target = evt_obj.target
	
	if multiTarget:
		#No bonus if the list is non empty
		if effectedList:
			return 0
	else:
		#Check if this opponent has been effected by this casting
		if target in effectedList:
			return 0
	
	int = attachee.stat_level_get(stat_intelligence)
	intMod = (int - 10)/2
	
	#Increase warmage edge damage on a critical hit
	if evt_obj.damage_packet.critical_multiplier > 1:
		intMod = intMod * 2
	
	if intMod > 0:
		evt_obj.damage_packet.bonus_list.add_from_feat(intMod, 0, 137, "Warmage Edge")
		
	args.set_arg(0, spellID)
	effectedList.append(target)
		
	#Add the target to the list of targets effected this round
	return 0
		
warmageEdge = PythonModifier("Warmage Edge", 2) #Previous Spell ID, Spare
warmageEdge.MapToFeat("Warmage Edge")
warmageEdge.AddHook(ET_OnDispatchSpellDamage, EK_NONE, WarmageEdgeOnSpellDamage, ())
warmageEdge.AddHook(ET_OnBeginRound, EK_NONE, WarmageBeginRound, ())

#Armored Mage

def WarmageSpellFailure(attachee, args, evt_obj):
	#Only effects spells cast as a warmage
	if evt_obj.data1 != classEnum:
		return 0

	equip_slot = evt_obj.data2
	item = attachee.item_worn_at(equip_slot)

	if item == OBJ_HANDLE_NULL:
		return 0
		
	if equip_slot == item_wear_armor: # warmage can cast in light armor (and medium armor at level 8 or greater) with no spell failure
		warmageLevel = attachee.stat_level_get(stat_level_warmage)
		armor_flags = item.obj_get_int(obj_f_armor_flags)
		if (armor_flags & ARMOR_TYPE_NONE) or (armor_flags == ARMOR_TYPE_LIGHT) or ((armor_flags == ARMOR_TYPE_MEDIUM) and (warmageLevel > 7)):
			return 0
	
	if equip_slot == item_wear_shield:  # warmage can cast with a light shield (or buclker) with no spell failure
		shieldFailure = item.obj_get_int(obj_f_armor_arcane_spell_failure)
		if shieldFailure <= 5: #Light shields and bucklers have 5% spell failure
			return 0
			
	evt_obj.return_val += item.obj_get_int(obj_f_armor_arcane_spell_failure)
	return 0

armoredMage = PythonModifier("Warmage Armored Mage", 2) #Spare, Spare
armoredMage.MapToFeat("Warmage Armored Mage")
armoredMage.AddHook(ET_OnD20Query, EK_Q_Get_Arcane_Spell_Failure, WarmageSpellFailure, ())
