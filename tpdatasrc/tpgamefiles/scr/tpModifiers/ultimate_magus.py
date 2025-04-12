from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

###################################################

def GetConditionName():
	return "Ultimate Magus"

def GetSpellCasterConditionName():
	return "Ultimate Magus Spellcasting"
	
print "Registering " + GetConditionName()

classEnum = stat_level_ultimate_magus
classSpecModule = __import__('class088_ultimate_magus')

###################################################

expandedSpellKnowledgeEnum = 8800
augmentedCastingEnum = 8801

ACFeats = [feat_quicken_spell, feat_heighten_spell, feat_empower_spell, feat_maximize_spell, feat_widen_spell, feat_enlarge_spell, feat_extend_spell, feat_silent_spell, feat_still_spell]

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

# Ultimate Magus raises the caster level for its two base classes specified in Modifier args 0 & 1, the bonus for each is in args 2 & 3

# configure the spell casting condition to hold the highest two Arcane Spontaneous/Vancian classes as chosen-to-be-extended classes
def OnAddSpellCasting(attachee, args, evt_obj):

	#arg0 holds the vancian class, arg1 holds the spontaneous class
	if args.get_arg(0) == 0:
		vancianClass = char_class_utils.GetHighestVancianArcaneClass(attachee)
		spontaneousClass = char_class_utils.GetHighestSpontaneousArcaneClass(attachee)
		vacianLvl = attachee.stat_level_get( vancianClass )
		spontaneousLvl = attachee.stat_level_get( spontaneousClass )

		levelUpVacian, levelupSpontaneous = classSpecModule.CheckUMIncreaseOnLevelup(1, vacianLvl, spontaneousLvl)
		
		args.set_arg(0, vancianClass)
		args.set_arg(1, spontaneousClass)
		args.set_arg(2, levelUpVacian)
		args.set_arg(3, levelupSpontaneous)
	
	return 0

def OnGetBaseCasterLevel(attachee, args, evt_obj):
	class_extended_vancian = args.get_arg(0)
	class_extended_spontaneous = args.get_arg(1)
	classLvl = attachee.stat_level_get(classEnum)
	class_code = evt_obj.arg0
	
	if class_code == class_extended_vancian:
		bonus = args.get_arg(2)
		evt_obj.bonus_list.add(bonus, 0, 137)
	elif class_code == class_extended_spontaneous:
		bonus = args.get_arg(3)
		evt_obj.bonus_list.add(bonus, 0, 137)
	else:
		if evt_obj.arg1 != 0:# are you specifically looking for the Ultimate Magus caster level?
			bonus = attachee.stat_level_get(classEnum)
			evt_obj.bonus_list.add(bonus, 0, 137)
	return 0

def OnSpellListExtensionGet(attachee, args, evt_obj):
	class_extended_vancian = args.get_arg(0)
	class_extended_spontaneous = args.get_arg(1)
	classLvl = attachee.stat_level_get(classEnum)
	class_code = evt_obj.arg0
	
	if class_code == class_extended_vancian:
		bonus = args.get_arg(2)
		evt_obj.bonus_list.add(bonus, 0, 137)
	elif class_code == class_extended_spontaneous:
		bonus = args.get_arg(3)
		evt_obj.bonus_list.add(bonus, 0, 137)
	else:
		if evt_obj.arg1 != 0:# are you specifically looking for the Ultimate Magus caster level?
			bonus = attachee.stat_level_get(classEnum)
			evt_obj.bonus_list.add(bonus, 0, 137)
	return 0

def OnInitLevelupSpellSelection(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	class_extended_vancian = args.get_arg(0)
	class_extended_spontaneous = args.get_arg(1)
	caster_level_vancian = args.get_arg(2)
	caster_level_spontaneous = args.get_arg(3)
	classSpecModule.InitSpellSelection(attachee, class_extended_vancian, class_extended_spontaneous, caster_level_vancian, caster_level_spontaneous)
	return 0

def OnLevelupSpellsCheckComplete(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	class_extended_vancian = args.get_arg(0)
	class_extended_spontaneous = args.get_arg(1)
	caster_level_vancian = args.get_arg(2)
	caster_level_spontaneous = args.get_arg(3)
	if not classSpecModule.LevelupCheckSpells(attachee, class_extended_vancian, class_extended_spontaneous, caster_level_vancian, caster_level_spontaneous):
		evt_obj.bonus_list.add(-1, 0, 137) # denotes incomplete spell selection
	return 1

def OnLevelupSpellsFinalize(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	class_extended_vancian = args.get_arg(0)
	class_extended_spontaneous = args.get_arg(1)
	caster_level_vancian = args.get_arg(2)
	caster_level_spontaneous = args.get_arg(3)
	levelUpVacian, levelUpSpontaneous = classSpecModule.LevelupSpellsFinalize(attachee, class_extended_vancian, class_extended_spontaneous, caster_level_vancian, caster_level_spontaneous)
	args.set_arg(2, levelUpVacian)
	args.set_arg(3, levelUpSpontaneous)
	return 0
	
def GetUMVacianClass(attachee, args, evt_obj):
	evt_obj.return_val = args.get_arg(0)
	return 0
	
def GetUMSpontaneousClass(attachee, args, evt_obj):
	evt_obj.return_val = args.get_arg(1)
	return 0

spellCasterSpecObj = PythonModifier(GetSpellCasterConditionName(), 8) #Vancian Class , Spontaneous Class, Vancian Bonus, Spontaneous Bonus
spellCasterSpecObj.AddHook(ET_OnConditionAdd, EK_NONE, OnAddSpellCasting, ())
spellCasterSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
spellCasterSpecObj.AddHook(ET_OnSpellListExtensionGet, EK_NONE, OnSpellListExtensionGet, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Activate, OnInitLevelupSpellSelection, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Check_Complete, OnLevelupSpellsCheckComplete, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())
spellCasterSpecObj.AddHook(ET_OnD20PythonQuery, "UM Spontaneous Class", GetUMSpontaneousClass, ())
spellCasterSpecObj.AddHook(ET_OnD20PythonQuery, "UM Vacian Class", GetUMVacianClass, ())

# Arcane Spell Power
def ArcaneSpellPowerUMBonus(attachee, args, evt_obj):
	#Must be arcane spell
	if not attachee.is_arcane_spell_class(evt_obj.arg0):
		return 0

	umLevel = attachee.stat_level_get(stat_level_ultimate_magus)
	
	if umLevel < 4:
		bonusValue = 1
	elif umLevel < 7:
		bonusValue = 2
	elif umLevel < 10:
		bonusValue = 3
	else:
		bonusValue = 4
	
	if bonusValue > 0:
		evt_obj.bonus_list.add(bonusValue, 0, "Arcane Spell Power")
	
	return 0

umSpellPower = PythonModifier("Arcane Spell Power", 2) #Spare, Spare
umSpellPower.MapToFeat("Arcane Spell Power")
umSpellPower.AddHook(ET_OnSpellCasterGeneral, EK_SPELL_Base_Caster_Level_2, ArcaneSpellPowerUMBonus, ())

def OnAddExpandedSpellKnowledge(attachee, args, evt_obj):
	args.set_arg(0,1)
	args.set_arg(1,1)
	args.set_arg(2,1)
	args.set_arg(3,1)
	args.set_arg(4,1)
	return 0

def ExpandedSpellKnowledgeRadial(attachee, args, evt_obj):
	umLevel = attachee.stat_level_get(stat_level_ultimate_magus)
	halfUmLevel = int(umLevel/2)
	
	available = []
	for i in range(1, halfUmLevel + 1):
		arg = args.get_arg(i-1)
		if arg > 0:
			available.append(i)
	
	#Add menu item for each level
	for level in available:
		menuName = "Expanded Spell Knowledge " + str(level)
		radialParent = tpdp.RadialMenuEntryParent(menuName)
		expandedSpellKnowledgeID = radialParent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
			
		# create the spell level nodes
		spell_level_ids = []
		
		#0 to the max level
		for spellLevel in range(0,level+1):
			spell_level_node = tpdp.RadialMenuEntryParent(str(spellLevel))
			spell_level_ids.append( spell_level_node.add_as_child(attachee, expandedSpellKnowledgeID) )

		spontaneousCastingClass = attachee.d20_query("UM Spontaneous Class")

		known_spells = attachee.spells_known
		
		#Find all known spontaneous spells arcane so we don't add them to the selectable spells
		spontaneous_known = []
		for knSp in known_spells:
			if (knSp.caster_class == spontaneousCastingClass) and (knSp.caster_class > 0) and attachee.is_arcane_spell_class(knSp.caster_class):
				spontaneous_known.append(knSp.spell_enum)
		
		#Add each arcane spell under the level that is not alreay in the spontaneous spell list
		for knSp in known_spells:
			if knSp.spell_level <= level and (knSp.caster_class > 0) and attachee.is_arcane_spell_class(knSp.caster_class):
				if knSp.spell_enum not in spontaneous_known:
					spell_node = tpdp.RadialMenuEntryPythonAction(knSp, D20A_PYTHON_ACTION, expandedSpellKnowledgeEnum, level)
					spell_node.add_as_child(attachee, spell_level_ids[knSp.spell_level])
	return 0

def ExpandedSpellKnowledgeAddSpell(attachee, args, evt_obj):
	spellEnum = evt_obj.d20a.spell_data.spell_enum
	spellLevel = evt_obj.d20a.spell_data.get_spell_level()
	spellName = evt_obj.d20a.spell_data.get_spell_name()
	classCode = attachee.d20_query("UM Spontaneous Class")
	maxLevel = evt_obj.d20a.data1
	argNum = maxLevel - 1
	
	#Add the new spell to the spontaneous class
	attachee.spell_known_add_to_char_class(spellEnum, classCode, spellLevel)
	
	#Use up the spell knowledge slot
	args.set_arg(argNum, 0)
	
	attachee.float_text_line("Expanded Spell Knowledge " + str(maxLevel) + " Added:  " + spellName)
	return 0

umlExpandedSpellKnowledge = PythonModifier("Expanded Spell Knowledge", 6) #Level 1 Used, Level 2 Used, Level 3 Used, Level 4 Used, Level 5 Used, Spare
umlExpandedSpellKnowledge.MapToFeat("Expanded Spell Knowledge")
umlExpandedSpellKnowledge.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, ExpandedSpellKnowledgeRadial, ())
umlExpandedSpellKnowledge.AddHook(ET_OnD20PythonActionPerform, expandedSpellKnowledgeEnum, ExpandedSpellKnowledgeAddSpell, ())
umlExpandedSpellKnowledge.AddHook(ET_OnConditionAdd, EK_NONE, OnAddExpandedSpellKnowledge, ())

def AugmentCastingNewDay(attachee, args, evt_obj):
	uses = int(attachee.stat_level_get(classEnum) / 2) + 3
	args.set_arg(0, uses)
	return 0

def AugmentCastingEffectCheck(attachee, args, evt_obj):
	#Check for remaining uses
	if args.get_arg(0) < 1:
		evt_obj.return_val = AEC_OUT_OF_CHARGES
		return 0
	return 1

def AugmentCastingRadialMenuEntry(attachee, args, evt_obj):
	radialParent = tpdp.RadialMenuEntryParent("Augment Casting")
	arcaneBoostID = radialParent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)

	spontaneousCastingClass = attachee.d20_query("UM Spontaneous Class")
	vacianCastingClass = attachee.d20_query("UM Vacian Class")

	for classID in [spontaneousCastingClass, vacianCastingClass]:
		#Add the effect to the arcane boost id menu
		className = tpdp.get_stat_short_name(classID)
		
		effectNode = tpdp.RadialMenuEntryParent(className)
		spellTypeID = effectNode.add_as_child(attachee, arcaneBoostID)
		
		for feat in ACFeats:
			if attachee.has_feat(feat):
				featName = tpdp.get_stat_name(1000+feat)
				featNode = tpdp.RadialMenuEntryParent(featName)
				featNodeID = featNode.add_as_child(attachee, spellTypeID)
				
				#Figure out the minimum level needed for the spell
				minLevel = 1 #default
				if feat == feat_quicken_spell:
					minLevel = 4
				elif feat == feat_maximize_spell:
					minLevel = 3
				elif feat == feat_widen_spell:
					minLevel = 3
				elif feat == feat_empower_spell:
					minLevel = 2
				
				# create the spell level nodes
				spell_level_ids = []
			
				#Add the name for the arcane boost effect type sub menu
				for spellLevel in range(minLevel,10):
					spell_level_node = tpdp.RadialMenuEntryParent(str(spellLevel))
					spell_level_ids.append( spell_level_node.add_as_child(attachee, featNodeID) )

				if classID == spontaneousCastingClass:
					known_spells = attachee.spells_known
					for knSp in known_spells:
						if knSp.is_naturally_cast() and (knSp.spell_level >= minLevel) and attachee.spontaneous_spells_remaining(knSp.spell_class, knSp.spell_level):
							spell_node = tpdp.RadialMenuEntryPythonAction(knSp, D20A_PYTHON_ACTION, augmentedCastingEnum, feat)
							spell_node.add_as_child(attachee, spell_level_ids[knSp.spell_level-minLevel])
				else:
					mem_spells = attachee.spells_memorized
					for memSp in mem_spells:
						if (not memSp.is_used_up()) and (not memSp.is_naturally_cast()) and (memSp.spell_level >= minLevel):
							spell_node = tpdp.RadialMenuEntryPythonAction(memSp, D20A_PYTHON_ACTION, augmentedCastingEnum, feat)
							spell_node.add_as_child(attachee, spell_level_ids[memSp.spell_level-minLevel])
	return 0
	
def AugmentCastingD20PythonActionPerform(attachee, args, evt_obj):
	uses = args.get_arg(0)
	if uses < 1:
		return 0
	uses = uses - 1
	args.set_arg(0, uses)
	
	spell_packet = tpdp.SpellPacket(attachee, evt_obj.d20a.spell_data)
	
	#Count the spell as used up
	spell_packet.debit_spell()
	
	spellCastingClass = spell_packet.get_spell_casting_class()
	feat = evt_obj.d20a.data1
	level = spell_packet.spell_known_slot_level
	attachee.condition_add_with_args("Augmented Casting Condition", feat, spellCastingClass, level)

	return 0


umAugmentedCasting = PythonModifier("Augmented Casting", 3) #Uses, Spare, Spare
umAugmentedCasting.MapToFeat("Augmented Casting")
umAugmentedCasting.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, AugmentCastingRadialMenuEntry, ())
umAugmentedCasting.AddHook(ET_OnD20PythonActionCheck, augmentedCastingEnum, AugmentCastingEffectCheck, ())
umAugmentedCasting.AddHook(ET_OnD20PythonActionPerform, augmentedCastingEnum, AugmentCastingD20PythonActionPerform, ())
umAugmentedCasting.AddHook(ET_OnConditionAdd, EK_NONE, AugmentCastingNewDay, ())
umAugmentedCasting.AddHook(ET_OnNewDay, EK_NEWDAY_REST, AugmentCastingNewDay, ())

def AugmentCastingConditionEffectTooltip(attachee, args, evt_obj):
	evt_obj.append("Augment Casting") 
	return 0

def AugmentCastingConditionEffectTooltipEffect(attachee, args, evt_obj):
	feat = args.get_arg(0)
	classID = args.get_arg(1)
	featString =  tpdp.get_stat_name(1000+feat)
	if feat == feat_heighten_spell:
		level = args.get_arg(2)
		featString = featString + " " + str(level)
	extraString = "(" + tpdp.get_stat_short_name(classID) + ") " + featString
	evt_obj.append(tpdp.hash("AUGMENT_CASTING"), -2, extraString)
	return 0

def AugmentCastingConditionRemove(attachee, args, evt_obj):
	args.condition_remove()
	return 0

def AugmentCastingMetaMagicUpdate(attachee, args, evt_obj):
	#Check that it is arcane
	if not evt_obj.is_arcane_spell():
		return 0
	
	#Can't use the same class that the spell was deducted from
	spellCastingClass = args.get_arg(1)
	if spellCastingClass == evt_obj.get_spell_casting_class():
		return 0
		
	#Feat cannot be used on a spell higher than half the ultimate magus level
	umLevel = attachee.stat_level_get(stat_level_ultimate_magus)
	halfUmLevel = int(umLevel/2)
	if evt_obj.spell_level > halfUmLevel:
		return 0
		
	feat = args.get_arg(0)

	#Apply the appropriate meta magic once only
	if feat == feat_empower_spell:
		if evt_obj.meta_magic.get_empower_count() < 1:  #Only once
			evt_obj.meta_magic.set_empower_count(1)
		else:
			return #No effect, don't set the cost
	elif feat == feat_maximize_spell:
		if not evt_obj.meta_magic.get_maximize():    #Only once
			evt_obj.meta_magic.set_maximize(true)
		else:
			return #No effect, don't set the cost
	elif feat == feat_heighten_spell:
		level = args.get_arg(2)
		maxHeighten =  9 - evt_obj.spell_level #Heighten to level 9 always
		heightenCount = min(level, maxHeighten)
		if heightenCount > 0:
			evt_obj.meta_magic.set_heighten_count(heightenCount)
	elif feat == feat_extend_spell:
		if evt_obj.meta_magic.get_extend_count() < 1:  #Only once
			evt_obj.meta_magic.set_extend_count(1)
		else:
			return #No effect, don't set the cost
	elif feat == feat_widen_spell:
		if evt_obj.meta_magic.get_widen_count() < 1:  #Only once
			evt_obj.meta_magic.set_widen_count(1)
		else:
			return #No effect, don't set the cost
	elif feat == feat_enlarge_spell:
		if evt_obj.meta_magic.get_enlarge_count() < 1:  #Only once
			evt_obj.meta_magic.set_enlarge_count(1)
		else:
			return #No effect, don't set the cost
	elif feat == feat_still_spell:
		if not evt_obj.meta_magic.get_still():  #Only once
			evt_obj.meta_magic.set_still(true)
		else:
			return #No effect, don't set the cost
	elif feat == feat_silent_spell:
		if not evt_obj.meta_magic.get_silent():  #Only once
			evt_obj.meta_magic.set_silent(true)
		else:
			return #No effect, don't set the cost
	elif feat == feat_quicken_spell:
		if evt_obj.meta_magic.get_quicken() < 1:  #Only once
			evt_obj.meta_magic.set_quicken(1)
		else:
			return #No effect, don't set the cost
		
	return 0

umAugmentedCastingCondition = PythonModifier("Augmented Casting Condition", 5) #Feat to Apply, Class that Sacrificed Spell, Spare, Spare, Spare
umAugmentedCastingCondition.AddHook(ET_OnGetTooltip, EK_NONE, AugmentCastingConditionEffectTooltip, ())
umAugmentedCastingCondition.AddHook(ET_OnGetEffectTooltip, EK_NONE, AugmentCastingConditionEffectTooltipEffect, ())
umAugmentedCastingCondition.AddHook(ET_OnNewDay, EK_NEWDAY_REST, AugmentCastingConditionRemove, ())  #Condition won't go past a day
umAugmentedCastingCondition.AddHook(ET_OnMetaMagicMod, EK_NONE, AugmentCastingMetaMagicUpdate, ())
umAugmentedCastingCondition.AddHook(ET_OnD20PythonSignal, "Sudden Metamagic Deduct Charge", AugmentCastingConditionRemove, ())

