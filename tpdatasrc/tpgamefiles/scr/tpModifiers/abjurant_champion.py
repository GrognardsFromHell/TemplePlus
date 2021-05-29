from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils
import tpactions

###################################################

def GetConditionName():
	return "Abjurant Champion"

def GetSpellCasterConditionName():
	return "Abjurant Champion Spellcasting"
	
print "Registering " + GetConditionName()

classEnum = stat_level_abjurant_champion
arcaneBoostEnum = 2900

classSpecModule = __import__('class045_abjurant_champion')
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

##### Spell casting

# configure the spell casting condition to hold the highest Arcane classs
def OnAddSpellCasting(attachee, args, evt_obj):
	#arg0 holds the arcane class
	if (args.get_arg(0) == 0):
		args.set_arg(0, char_class_utils.GetHighestArcaneClass(attachee))
	return 0

# Extend caster level for base casting class
def OnGetBaseCasterLevel(attachee, args, evt_obj):
	class_extended_1 = args.get_arg(0)
	class_code = evt_obj.arg0
	if (class_code != class_extended_1):
		if (evt_obj.arg1 == 0): # arg1 != 0 means you're looking for this particular class's contribution
			return 0
	classLvl = attachee.stat_level_get(classEnum)
	if (classLvl == 0):
		return 0
	evt_obj.bonus_list.add(classLvl, 0, 137)
	return 0

def OnSpellListExtensionGet(attachee, args, evt_obj):
	class_extended_1 = args.get_arg(0)
	class_code = evt_obj.arg0
	if (class_code != class_extended_1):
		if (evt_obj.arg1 == 0): # arg1 != 0 means you're looking for this particular class's contribution
			return 0
	classLvl = attachee.stat_level_get(classEnum)
	if (classLvl == 0):
		return 0
	evt_obj.bonus_list.add(classLvl, 0, 137)
	return 0

def OnInitLevelupSpellSelection(attachee, args, evt_obj):
	if (evt_obj.arg0 != classEnum):
		return 0
	classLvl = attachee.stat_level_get(classEnum)
	if (classLvl == 0):
		return 0
	class_extended_1 = args.get_arg(0)
	classSpecModule.InitSpellSelection(attachee, class_extended_1)
	return 0

def OnLevelupSpellsCheckComplete(attachee, args, evt_obj):
	if (evt_obj.arg0 != classEnum):
		return 0
	class_extended_1 = args.get_arg(0)
	if (not classSpecModule.LevelupCheckSpells(attachee, class_extended_1) ):
		evt_obj.bonus_list.add(-1, 0, 137) # denotes incomplete spell selection
	return 1
	
def OnLevelupSpellsFinalize(attachee, args, evt_obj):
	if (evt_obj.arg0 != classEnum):
		return 0
	classLvl = attachee.stat_level_get(classEnum)
	if (classLvl == 0):
		return 0
	class_extended_1 = args.get_arg(0)
	classSpecModule.LevelupSpellsFinalize(attachee, class_extended_1)
	return

spellCasterSpecObj = PythonModifier(GetSpellCasterConditionName(), 8)
spellCasterSpecObj.AddHook(ET_OnConditionAdd, EK_NONE, OnAddSpellCasting, ())
spellCasterSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
spellCasterSpecObj.AddHook(ET_OnSpellListExtensionGet, EK_NONE, OnSpellListExtensionGet, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Activate, OnInitLevelupSpellSelection, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Check_Complete, OnLevelupSpellsCheckComplete, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())

#Abjurant Armor

#Returns the bonus to armor or shield spells.  It is queried by the engine.
def AbjurationShieldArmorBonus(attachee, args, evt_obj):
	evt_obj.return_val = attachee.stat_level_get(classEnum)
	return 0

abjurantChampionAbjurantArmor = PythonModifier("Abjurant Armor", 2) #Spare, Spare
abjurantChampionAbjurantArmor.MapToFeat("Abjurant Armor")
abjurantChampionAbjurantArmor.AddHook(ET_OnD20PythonQuery, "Abjuration Spell Shield Bonus", AbjurationShieldArmorBonus, ())
abjurantChampionAbjurantArmor.AddHook(ET_OnD20PythonQuery, "Abjuration Spell Armor Bonus", AbjurationShieldArmorBonus, ())

#Extended Abjuration
def ExtendAbjurationMetamagicUpdate(attachee, args, evt_obj):
	#Check for abjuration school
	spEntry = tpdp.SpellEntry(evt_obj.spell_enum)
	if spEntry.spell_school_enum != Abjuration:
		return 0
	
	#Get the metamagic info
	metaMagicData = evt_obj.meta_magic
	
	#Don't Extend more than once
	if metaMagicData.get_extend_count() < 1:
		metaMagicData.set_extend_count(1)
		
	return 0

abjurantChampionExtendAbjuration = PythonModifier("Extended Abjuration", 2) #Spare, Spare
abjurantChampionExtendAbjuration.MapToFeat("Extended Abjuration")
abjurantChampionExtendAbjuration.AddHook(ET_OnMetaMagicMod, EK_NONE, ExtendAbjurationMetamagicUpdate, ())

#Swift Abjuration

def SwiftAbjurationMetamagicUpdate(attachee, args, evt_obj):
	#Check for abjuration school
	spEntry = tpdp.SpellEntry(evt_obj.spell_enum)
	if spEntry.spell_school_enum != Abjuration:
		return 0
		
	#Check for spell level less than half of class level rounded update
	maxLevel = attachee.stat_level_get(classEnum) + 1
	maxLevel = int(maxLevel/2)
	if evt_obj.spell_level > maxLevel:
		return 0
	
	#Get the metamagic info
	metaMagicData = evt_obj.meta_magic
	
	#Don't quicken more than once
	if metaMagicData.get_quicken() < 1:
		metaMagicData.set_quicken(1)
		
	return 0

abjurantChampionSwiftAbjuration = PythonModifier("Swift Abjuration", 2) #Spare, Spare
abjurantChampionSwiftAbjuration.MapToFeat("Swift Abjuration")
abjurantChampionSwiftAbjuration.AddHook(ET_OnMetaMagicMod, EK_NONE, SwiftAbjurationMetamagicUpdate, ())

#Swift Arcane Boost
def GetArcaneBoostDescription(effectType):
	if effectType == 1:
		return "Attack Bonus"
	elif effectType == 2:
		return "Damage Bonus"
	elif effectType == 3:
		return "AC Bonus"
	elif effectType == 4:
		return "Save Bonus"
	elif effectType == 5:
		return "Cold Res"
	elif effectType == 6:
		return "Fire Res"
	elif effectType == 7:
		return "Electricity Res"
	elif effectType == 8:
		return "Acid Res"
	else:
		return "Sonic Res"
	

def AbjurantChampionArcaneBoostRadial(attachee, args, evt_obj):
	radialParent = tpdp.RadialMenuEntryParent("Arcane Boost")
	arcaneBoostID = radialParent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)

	#Iterate over all effect types ()
	for effectType in range(1,10):
		effectName = GetArcaneBoostDescription(effectType)
		
		#Add the effect to the arcane boost id menu
		effectNode = tpdp.RadialMenuEntryParent(effectName)
		effectID = effectNode.add_as_child(attachee, arcaneBoostID)
		
		# create the spell level nodes (level must be greater than 1)
		spell_level_ids = []
		
		#Add the name for the arcane boost effect type sub menu
		for p in range(1,10):
			spell_level_node = tpdp.RadialMenuEntryParent(str(p))
			spell_level_ids.append( spell_level_node.add_as_child(attachee, effectID) )

		known_spells = attachee.spells_known

		for knSp in known_spells:
			if knSp.is_naturally_cast() and (knSp.spell_level > 0):
				spell_node = tpdp.RadialMenuEntryPythonAction(knSp, D20A_PYTHON_ACTION, arcaneBoostEnum, effectType)
				spell_node.add_as_child(attachee, spell_level_ids[knSp.spell_level-1])

		mem_spells = attachee.spells_memorized
		for memSp in mem_spells:
			if (not memSp.is_used_up()) and (memSp.spell_level > 0):
				spell_node = tpdp.RadialMenuEntryPythonAction(memSp, D20A_PYTHON_ACTION, arcaneBoostEnum, effectType)
				spell_node.add_as_child(attachee, spell_level_ids[memSp.spell_level-1])
	return 0
	
def AbjurantChampionArcaneBoostPerform(attachee, args, evt_obj):
	cur_seq = tpactions.get_cur_seq()
	
	#Create the effect to count the spell as used
	cur_seq.spell_packet.debit_spell()
	
	type = evt_obj.d20a.data1
	amount = 0
	if type == 1 or type == 3 or type ==4:
		amount = cur_seq.spell_packet.spell_known_slot_level #Hit, AC, Save Equal to Spell Level
	elif type == 2:
		amount = 2 * cur_seq.spell_packet.spell_known_slot_level #Damage Equal to Twice Spell Level
	else:
		amount = 5 * cur_seq.spell_packet.spell_known_slot_level #Energy Resistance Equal to Five Times Spell Level
	
	#Add the effect with two parameters:  effect type and effect bonus
	attachee.condition_add_with_args("Arcane Boost Effect", type, amount, 0, 0)
	
	return 0
	
abjurantChampionArcaneBoost = PythonModifier("Arcane Boost", 3) #Spare, Spare, Spare
abjurantChampionArcaneBoost.MapToFeat("Arcane Boost")
abjurantChampionArcaneBoost.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, AbjurantChampionArcaneBoostRadial, ())
abjurantChampionArcaneBoost.AddHook(ET_OnD20PythonActionPerform, arcaneBoostEnum, AbjurantChampionArcaneBoostPerform, ())

def ArcaneBoostBeginRound(attachee, args, evt_obj):
	args.condition_remove() #Always disapears at the begining of the round
	return 0
	
def ArcaneBoostEffectTooltip(attachee, args, evt_obj):
	evt_obj.append("Arcane Boost")
	return 0

def ArcaneBoostEffectTooltipEffect(attachee, args, evt_obj):
	# Set the tooltip
	type = args.get_arg(0)
	bonus = args.get_arg(1)
	extraString = GetArcaneBoostDescription(type)
	if type < 5:
		extraString = extraString + " +"
	else:
		extraString = extraString + " DR"
	extraString = extraString + str(bonus)
	evt_obj.append(tpdp.hash("ARCANE_BOOST"), -2, extraString)
	return 0
	
def ArcaneBoostACBonus(attachee, args, evt_obj):
	type = args.get_arg(0)
	if type != 3:
		return 0
	bonus = args.get_arg(1)
	evt_obj.bonus_list.add(bonus , 0, "Arcane Boost") # Insight Bonus
	return 0
	
def ArcaneBoostSaveBonus(attachee, args, evt_obj):
	type = args.get_arg(0)
	if type != 4:
		return 0
	bonus = args.get_arg(1)
	evt_obj.bonus_list.add(bonus, 0, "Arcane Boost") # Racial Bonus
	return 0
	
def ArcaneBoostToHit(attachee, args, evt_obj):
	type = args.get_arg(0)
	if type != 1:
		return 0
	bonus = args.get_arg(1)
	evt_obj.bonus_list.add_from_feat(bonus, 0, 114, "Arcane Boost")
	return 0
	
def ArcaneBoostDamage(attachee, args, evt_obj):
	type = args.get_arg(0)
	if type != 2:
		return 0
	bonus = args.get_arg(1)
	evt_obj.damage_packet.bonus_list.add_from_feat(bonus, 0, 114, "Arcane Boost")
	return 0
	
def OnGetDamageResistance(attachee, args, evt_obj):
	type = args.get_arg(0)
	if type == 5:
		damType = D20DT_COLD
	elif type == 6:
		damType = D20DT_FIRE
	elif type == 7:
		damType = D20DT_ELECTRICITY
	elif type == 8:
		damType = D20DT_ACID
	elif type == 9:
		damType = D20DT_SONIC
	else: 
		return 0
	bonus = args.get_param(1)
	DAMAGE_MES_RESISTANCE_TO_ENERGY = 124
	evt_obj.damage_packet.add_damage_resistance(bonus, damType, DAMAGE_MES_RESISTANCE_TO_ENERGY)
	return 0
	
#Arcane Boost Effect
ArcaneBoostEffect = PythonModifier("Arcane Boost Effect", 4) #Bonus Type, Bonus Amount, Spare, Spare
ArcaneBoostEffect.AddHook(ET_OnBeginRound, EK_NONE, ArcaneBoostBeginRound, ())
ArcaneBoostEffect.AddHook(ET_OnGetTooltip, EK_NONE, ArcaneBoostEffectTooltip, ())
ArcaneBoostEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, ArcaneBoostEffectTooltipEffect, ())
ArcaneBoostEffect.AddHook(ET_OnGetAC, EK_NONE, ArcaneBoostACBonus, ())
ArcaneBoostEffect.AddHook(ET_OnSaveThrowLevel, EK_NONE, ArcaneBoostSaveBonus, ())
ArcaneBoostEffect.AddHook(ET_OnToHitBonus2, EK_NONE, ArcaneBoostToHit, ())
ArcaneBoostEffect.AddHook(ET_OnDealingDamage, EK_NONE, ArcaneBoostDamage, ())
ArcaneBoostEffect.AddHook(ET_OnTakingDamage, EK_NONE, OnGetDamageResistance, ())


#Martial Arcanist
def MartialArcanistOnGetBaseCasterLevel(attachee, args, evt_obj):
	#Bonus applies to the highest level arcane class
	spellPkt = evt_obj.get_spell_packet()
	castingClass = spellPkt.get_spell_casting_class()
	highestLevelArcaneClass = attachee.highest_arcane_class
	if castingClass != highestLevelArcaneClass:
		return 0

	#Add to make caster level equal to the base attack bonus if it is less
	bab = attachee.get_base_attack_bonus()
	casterLevel = attachee.highest_arcane_caster_level
	casterBonus = bab - casterLevel
	if casterBonus > 0:
		evt_obj.return_val += casterBonus
	
	return 0

abjurantChampionMartialArcanist = PythonModifier("Martial Arcanist", 2) #Spare, Spare
abjurantChampionMartialArcanist.MapToFeat("Martial Arcanist")
abjurantChampionMartialArcanist.AddHook(ET_OnGetCasterLevelMod, EK_NONE, MartialArcanistOnGetBaseCasterLevel, ())

