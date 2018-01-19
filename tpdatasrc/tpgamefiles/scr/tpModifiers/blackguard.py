from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

###################################################

def GetConditionName():
	return "Blackguard"

# def GetSpellCasterConditionName():
	# return "Blackguard Spellcasting"
	
print "Registering " + GetConditionName()

classEnum = stat_level_blackguard
classSpecModule = __import__('class022_blackguard')
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


def BlackguardSneakAttackDice(attachee, args, evt_obj):
	blg_lvl = attachee.stat_level_get(classEnum)
	palLvl = attachee.stat_level_get(stat_level_paladin)

	if blg_lvl < 4 and palLvl < 5:
		return 0
	evt_obj.return_val += (blg_lvl-1) /3
	if palLvl >= 5:
		evt_obj.return_val += 1
	return 0

def BlackguardRebukeUndeadLevel(attachee, args, evt_obj):
	if evt_obj.data1 != 1: # rebuke undead
		return 0
	blg_lvl = attachee.stat_level_get(classEnum)
	if blg_lvl < 3:
		return 0
	evt_obj.return_val += blg_lvl - 2
	return 0


def BlackguardFallenPaladin(attachee, args, evt_obj):
	palLvl = attachee.stat_level_get(stat_level_paladin)
	if palLvl:
		evt_obj.return_val = 1
	return 0

def BlackguardFallenIndicator(attachee, args, evt_obj):
	palLvl = attachee.stat_level_get(stat_level_paladin)
	if palLvl:
		evt_obj.append(175, -1, ": Blackguard")
	return 0

classSpecObj = PythonModifier(GetConditionName(), 0)
classSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())
classSpecObj.AddHook(ET_OnD20PythonQuery, "Sneak Attack Dice", BlackguardSneakAttackDice, ())
classSpecObj.AddHook(ET_OnD20PythonQuery, "Turn Undead Level", BlackguardRebukeUndeadLevel, ())
classSpecObj.AddHook(ET_OnD20Query, EK_Q_IsFallenPaladin, BlackguardFallenPaladin, ()) # forces "Fallen Paladin" status no matter what. There's no atoning for this one!
classSpecObj.AddHook(ET_OnGetEffectTooltip, EK_NONE, BlackguardFallenIndicator, ())

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
	return



# spellCasterSpecObj = PythonModifier(GetSpellCasterConditionName(), 8)
# spellCasterSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
classSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
classSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())



## Dark Blessing feat

def BlackguardDarkBlessing(attachee, args, evt_obj):
	#print "Dark Blessing save throw"
	cha_score = attachee.stat_level_get(stat_charisma)
	if cha_score < 10:
		return 0
	cha_mod = (cha_score-10)/2
	#print "adding bonus " + str(cha_mod)
	evt_obj.bonus_list.add_from_feat(cha_mod, 0, 114, "Dark Blessing")
	return 0

darkBless = PythonModifier("Dark Blessing Feat", 0)
darkBless.MapToFeat("Dark Blessing")
darkBless.AddHook(ET_OnSaveThrowLevel, EK_NONE, BlackguardDarkBlessing, ())


## Smite Good

def SmiteGoodReset(attachee, args, evt_obj):
	classLvl = attachee.stat_level_get(classEnum)
	palLvl = attachee.stat_level_get(stat_level_paladin)
	bonusFromPal = (palLvl + 3) / 4

	if classLvl < 2 and bonusFromPal == 0:
		return 0

	timesPerDay = 0
	if classLvl >= 2:
		timesPerDay += 1
	if classLvl >= 5:
		timesPerDay += 1
	if classLvl >= 10:
		timesPerDay += 1

	timesPerDay += bonusFromPal
	args.set_arg(0, timesPerDay)
	return 0

smiteGoodEnum = 2201
def SmiteGoodRadial(attachee, args, evt_obj):
	timesPerDay = args.get_arg(0)
	if timesPerDay <= 0:
		return 0
	radial_action = tpdp.RadialMenuEntryPythonAction("Smite Good", D20A_PYTHON_ACTION, smiteGoodEnum, 0, "TAG_BLACKGUARDS") # TODO add help entry
	radial_action.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
	return 0

def SmiteGoodCheck(attachee, args, evt_obj):
	timesPerDay = args.get_arg(0)
	if timesPerDay <= 0:
		evt_obj.return_val = AEC_OUT_OF_CHARGES
	return 0

def SmiteGoodPerform(attachee, args, evt_obj):
	d20a = evt_obj.d20a

	tgt = d20a.target

	ok_to_add = 0
	if tgt == OBJ_HANDLE_NULL:
		ok_to_add = 1
	else:
		if tgt.type == obj_t_npc or tgt.type == obj_t_pc:
			if d20a.query_can_be_affected_action_perform(tgt):
				tgt_alignment = tgt.obj_get_int(obj_f_critter_alignment)
				if tgt_alignment & ALIGNMENT_GOOD:
					ok_to_add = 1

	if ok_to_add:
		args.set_arg(0, args.get_arg(0) -  1) # decrease remaining usages
		attachee.condition_add("Smiting Good")
		return 0
	return 0

smiteGood = PythonModifier("Smite Good Feat", 2)
smiteGood.MapToFeat("Smite Good")
smiteGood.AddHook(ET_OnNewDay, EK_NEWDAY_REST, SmiteGoodReset, ())
smiteGood.AddHook(ET_OnConditionAdd, EK_NONE, SmiteGoodReset, ())
smiteGood.AddHook(ET_OnBuildRadialMenuEntry , EK_NONE, SmiteGoodRadial, ())
smiteGood.AddHook(ET_OnD20PythonActionPerform, smiteGoodEnum, SmiteGoodPerform, ())
smiteGood.AddHook(ET_OnD20PythonActionCheck, smiteGoodEnum, SmiteGoodCheck, ())

# Smiting Good effect
def SmitingGoodDamage(attachee, args, evt_obj):
	classLvl = attachee.stat_level_get(classEnum)
	tgt = evt_obj.attack_packet.target
	if tgt == OBJ_HANDLE_NULL:
		return 0
	ok_to_add = 0
	if tgt.type == obj_t_npc or tgt.type == obj_t_pc:
		tgt_alignment = tgt.obj_get_int(obj_f_critter_alignment)
		if tgt_alignment & ALIGNMENT_GOOD:
				ok_to_add = 1

	flags = evt_obj.attack_packet.get_flags()
	if flags & D20CAF_RANGED:
		ok_to_add = 0

	if ok_to_add:
		evt_obj.damage_packet.bonus_list.add_from_feat(classLvl, 0, 114, "Smite Good")
		args.condition_remove()
	return 0

def SmitingGoodToHit(attachee, args, evt_obj):
	cha_score = attachee.stat_level_get(stat_charisma)
	cha_mod = (cha_score - 10)/2
	tgt = evt_obj.attack_packet.target
	if tgt == OBJ_HANDLE_NULL:
		return 0
	ok_to_add = 0
	if tgt.type == obj_t_npc or tgt.type == obj_t_pc:
		tgt_alignment = tgt.obj_get_int(obj_f_critter_alignment)
		if tgt_alignment & ALIGNMENT_GOOD:
			ok_to_add = 1

	flags = evt_obj.attack_packet.get_flags()
	if flags & D20CAF_RANGED:
		ok_to_add = 0

	if not ok_to_add:
		return 0
	if cha_mod > 0:
		evt_obj.bonus_list.add_from_feat(cha_mod, 0, 114, "Smite Good")
	return 0

def SmitingGoodRemove(attachee, args, evt_obj):
	args.condition_remove()
	return 0

def SmitingGoodEffectTooltip(attachee, args, evt_obj):
	evt_obj.append(7, -2, "Smiting Good")
	return 0

smitingGoodEffect = PythonModifier("Smiting Good", 0)
smitingGoodEffect.AddHook(ET_OnDealingDamage, EK_NONE, SmitingGoodDamage, ())
smitingGoodEffect.AddHook(ET_OnToHitBonus2, EK_NONE, SmitingGoodToHit, ())
smitingGoodEffect.AddHook(ET_OnD20Signal, EK_S_Killed, SmitingGoodRemove, ())
smitingGoodEffect.AddHook(ET_OnBeginRound, EK_NONE, SmitingGoodRemove, ())
smitingGoodEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, SmitingGoodEffectTooltip, ())


# Aura of Despair

def AuraOfDespairBeginRound(attachee, args, evt_obj):
	ppl = game.obj_list_vicinity(attachee, OLC_CRITTERS)
	for o in ppl:
		if not o.allegiance_shared(attachee) and not o.is_friendly(attachee):
			o.condition_add("Despaired_Aura")
	return 0



def AuraOfDespairBegin(attachee, args, evt_obj):
	#print "Aura of Despair Begin"
	radius_feet = 10.0 + (attachee.radius / 12.0)
	#print "Effect radius (ft): " + str(radius_feet)
	obj_evt_id = attachee.object_event_append(OLC_CRITTERS, radius_feet)
	args.set_arg(2, obj_evt_id)
	#print "Aura of Despair: New Object Event ID: " + str(obj_evt_id)
	return 0

def AuraOfDespairAoEEntered(attachee, args, evt_obj):
	#print "Aura of Despair entered event"
	obj_evt_id = args.get_arg(2)

	if obj_evt_id != evt_obj.evt_id:
		#print "Aura of Despair Entered: ID mismatch " + str(evt_obj.evt_id) + ", stored was: " + str(obj_evt_id)
		return 0

	#print "Aura of Despair Entered, event ID: " + str(obj_evt_id)
	tgt = evt_obj.target
	if tgt == OBJ_HANDLE_NULL:
		return 0
	if attachee == OBJ_HANDLE_NULL:
		return 0

	if not tgt.allegiance_shared(attachee) and not tgt.is_friendly(attachee):
		tgt.condition_add_with_args("Despaired_Aura", 0, 0, obj_evt_id)
	return 0



auraDesp = PythonModifier("Feat Aura of Despair", 4)
auraDesp.MapToFeat("Aura of Despair")
auraDesp.AddHook(ET_OnConditionAdd, EK_NONE, AuraOfDespairBegin, ())
auraDesp.AddHook(ET_OnD20Signal, EK_S_Teleport_Reconnect, AuraOfDespairBegin, ())
auraDesp.AddHook(ET_OnObjectEvent, EK_OnEnterAoE, AuraOfDespairAoEEntered, ())
#auraDesp.AddHook(ET_OnBeginRound, EK_NONE, AuraOfDespairBeginRound, ())



def AuraDespairEffSavingThrow(attachee, args, evt_obj):
	#obj_evt_id = args.get_arg(2)
	evt_obj.bonus_list.add(-2, 0, "Aura of Despair")
	return 0

def AuraOfDespairAoEExited(attachee, args, evt_obj):
	obj_evt_id = args.get_arg(2)
	if obj_evt_id != evt_obj.evt_id:
		#print "Aura of Despair Exited: ID mismatch " + str(evt_obj.evt_id) + ", stored was: " + str(obj_evt_id)
		return 0
	#print "Aura of Despair (ID " + str(obj_evt_id) +") Exited, critter: " + attachee.description + " "
	args.condition_remove()
	return 0

def AuraDespTooltip(attachee, args, evt_obj):
	evt_obj.append("Despaired")
	return 0

def AuraOfDespairRemove(attachee, args, evt_obj):
	#print "Removing Aura of Despair Effect for " + attachee.description
	args.condition_remove()
	return 0

auraDespEffect = PythonModifier("Despaired_Aura", 4)
auraDespEffect.AddHook(ET_OnSaveThrowLevel, EK_NONE, AuraDespairEffSavingThrow, () )
auraDespEffect.AddHook(ET_OnObjectEvent, EK_OnLeaveAoE, AuraOfDespairAoEExited, ())
auraDespEffect.AddHook(ET_OnGetTooltip, EK_NONE, AuraDespTooltip, ())
auraDespEffect.AddHook(ET_OnNewDay, EK_NEWDAY_REST, AuraOfDespairRemove, ())
auraDespEffect.AddHook(ET_OnD20Signal, EK_S_Teleport_Prepare, AuraOfDespairRemove, ())