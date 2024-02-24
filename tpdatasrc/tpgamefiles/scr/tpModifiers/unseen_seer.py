from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_editor
import char_class_utils

###################################################

def GetConditionName():
	return "Unseen Seer"

def GetSpellCasterConditionName():
	return "Unseen Seer Spellcasting"
	
print "Registering " + GetConditionName()

classEnum = stat_level_unseen_seer
classSpecModule = __import__('class089_unseen_seer')
###################################################

unseenSeerAdvancedLearningEnum = 8900

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

def QueryUnseenSeerExtendedClass(attachee, args, evt_obj):
	class_extended_1 = args.get_arg(0)
	evt_obj.return_val = class_extended_1
	return 0

spellCasterSpecObj = PythonModifier(GetSpellCasterConditionName(), 8)
spellCasterSpecObj.AddHook(ET_OnConditionAdd, EK_NONE, OnAddSpellCasting, ())
spellCasterSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
spellCasterSpecObj.AddHook(ET_OnSpellListExtensionGet, EK_NONE, OnSpellListExtensionGet, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Activate, OnInitLevelupSpellSelection, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Check_Complete, OnLevelupSpellsCheckComplete, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())
spellCasterSpecObj.AddHook(ET_OnD20PythonQuery, "Unseen Seer Extended Class", QueryUnseenSeerExtendedClass, ())

def GetUnseenSeerDamageBonus(level):
	if level < 4:
		return 1
	elif level < 7:
		return 2
	elif level < 10:
		return 3
	return 4

def UnseenSeerDamageBonusSneakAttack(attachee, args, evt_obj):
	unseenSeerLvl = attachee.stat_level_get(classEnum)
	if evt_obj.data1 == classEnum: #class leveling up
		unseenSeerLvl = unseenSeerLvl + 1
	if unseenSeerLvl <= 0:
		return 0
	
	evt_obj.return_val += GetUnseenSeerDamageBonus(unseenSeerLvl)
	
	return 0
	
def UnseenSeerDamageBonusSneakScout(attachee, args, evt_obj):
	unseenSeerLvl = attachee.stat_level_get(classEnum)
	if evt_obj.data1 == classEnum: #class leveling up
		unseenSeerLvl = unseenSeerLvl + 1
	if unseenSeerLvl <= 0:
		return 0
	
	#Only apply to rogue if there are rogue levels also.  This is arbitrary I could allow the user to select later.
	rogueLevel = attachee.stat_level_get(stat_level_rogue)
	if rogueLevel > 0:
		return 0
	
	evt_obj.return_val += GetUnseenSeerDamageBonus(unseenSeerLvl)
	
	return 0
	
unseenSeerDamageBonusCondition = PythonModifier("Unseen Seer Damage bonus", 2) #Spare, Spare
unseenSeerDamageBonusCondition.MapToFeat("Unseen Seer Damage bonus")
unseenSeerDamageBonusCondition.AddHook(ET_OnD20PythonQuery, "Sneak Attack Dice", UnseenSeerDamageBonusSneakAttack, ())
unseenSeerDamageBonusCondition.AddHook(ET_OnD20PythonQuery, "Skirmish Additional Dice", UnseenSeerDamageBonusSneakScout, ())


def OnAddAdvancedLearning(attachee, args, evt_obj):
	extendedClass = attachee.d20_query("Unseen Seer Extended Class")
	effectiveLevel = attachee.get_level_for_spell_selection(extendedClass)
	
	#Args contain the effective class level or zero if not currently active.  At the beginning it will have the current level.
	spellLevel = game.get_max_spell_level(extendedClass, effectiveLevel)
	args.set_arg(0,spellLevel)
	args.set_arg(1,0)
	args.set_arg(2,0)
	
	return 0

def AdvancedLearningRadial(attachee, args, evt_obj):
	unseenSeerLevel = attachee.stat_level_get(stat_level_unseen_seer)
	extendedClass = attachee.d20_query("Unseen Seer Extended Class")
	
	#Add top level menu item for each advanced learning option
	for i in range(0, 3):
		maxLevel = args.get_arg(i)
		if maxLevel > 0:
			menuName = "Advanced Learning " + str(i+1)
			radialParent = tpdp.RadialMenuEntryParent(menuName)
			expandedSpellKnowledgeID = radialParent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
				
			#0 to the max level, add the level nodes
			spell_level_ids = []
			for spellLevel in range(0,maxLevel+1):
				spell_level_node = tpdp.RadialMenuEntryParent(str(spellLevel))
				spell_level_ids.append( spell_level_node.add_as_child(attachee, expandedSpellKnowledgeID) )

			#Go through all valid spells and add all divination spells under the max level that have not been added
			valid_spells = game.get_valid_spell_enums()
			for spell in valid_spells:
				spEntry = tpdp.SpellEntry(spell)
				minLevel = spEntry.get_lowest_spell_level()
				if minLevel <= maxLevel and spEntry.spell_school_enum == Divination:
					#Using | 0x80 changes from a stat to a class code
					if not attachee.is_spell_known(spell, extendedClass | 0x80):
						spStore = PySpellStore(spell, extendedClass, minLevel)
						spell_node = tpdp.RadialMenuEntryPythonAction(spStore, D20A_PYTHON_ACTION, unseenSeerAdvancedLearningEnum, i)
						spell_node.add_as_child(attachee, spell_level_ids[spStore.spell_level])

	return 0

def AdvancedLearningAddSpell(attachee, args, evt_obj):
	spellEnum = evt_obj.d20a.spell_data.spell_enum
	spellLevel = evt_obj.d20a.spell_data.get_spell_level()
	spellName = evt_obj.d20a.spell_data.get_spell_name()
	classCode = attachee.d20_query("Unseen Seer Extended Class")
	
	#Add the new spell to the unseen seer class
	attachee.spell_known_add_to_char_class(spellEnum, classCode, spellLevel)
	
	argNum = evt_obj.d20a.data1
	
	#Disable the advanced learning slot
	args.set_arg(argNum, 0)
	
	attachee.float_text_line("Advanced Learning " + str(argNum+1) + " Added spell: " + spellName)
	
	return 0
	
def AddAdvancedLearningSpell(attachee, args, evt_obj):
	extendedClass = attachee.d20_query("Unseen Seer Extended Class")
	casterLevel = attachee.stat_level_get(extendedClass)
	unseenSeerLevel = attachee.stat_level_get(classEnum) + 1
	effectiveLevel = attachee.get_level_for_spell_selection(extendedClass) + 1

	spellLevel = game.get_max_spell_level(extendedClass, effectiveLevel)
	
	if unseenSeerLevel == 5:
		args.set_arg(1,spellLevel)
	elif unseenSeerLevel == 8:
		args.set_arg(2,spellLevel)
	
	return 0

unseenSeerAdvancedLearningCondition = PythonModifier("Unseen Seer Advanced Learning",5 ) #Advanced Learning 1, Advanced Learning 2, Advanced Learning 3, Spare X2
unseenSeerAdvancedLearningCondition.MapToFeat("Unseen Seer Advanced Learning")
unseenSeerAdvancedLearningCondition.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, AdvancedLearningRadial, ())
unseenSeerAdvancedLearningCondition.AddHook(ET_OnD20PythonActionPerform, unseenSeerAdvancedLearningEnum, AdvancedLearningAddSpell, ())
unseenSeerAdvancedLearningCondition.AddHook(ET_OnConditionAdd, EK_NONE, OnAddAdvancedLearning, ())
unseenSeerAdvancedLearningCondition.AddHook(ET_OnD20PythonSignal, "Add Advanced Learning Spell", AddAdvancedLearningSpell, ())

# Divination Spell Power
def DivinationSpellPowerBonus(attachee, args, evt_obj):
	#Must be arcane spell
	spellPkt = evt_obj.get_spell_packet()
	spEntry = tpdp.SpellEntry(spellPkt.spell_enum)
	
	unseenSeerLvl = attachee.stat_level_get(classEnum)
	if unseenSeerLvl < 6:
		bonusValue = 1
	elif unseenSeerLvl < 9:
		bonusValue = 2
	else:
		bonusValue = 3
		
	if spEntry.spell_school_enum != Divination:
		bonusValue = bonusValue * -1
	
	evt_obj.return_val += bonusValue
	
	return 0
	
unseenSeerDivinationSpellPowerCondition = PythonModifier("Divination Spell Power", 2) #Spare, Spare
unseenSeerDivinationSpellPowerCondition.MapToFeat("Divination Spell Power")
unseenSeerDivinationSpellPowerCondition.AddHook(ET_OnGetCasterLevelMod, EK_NONE, DivinationSpellPowerBonus, ())