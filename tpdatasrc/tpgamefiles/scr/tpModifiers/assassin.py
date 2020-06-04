from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils
import tpactions

###################################################

def GetConditionName():
	return "Assassin"

# def GetSpellCasterConditionName():
	# return "Assassin Spellcasting"
	
print "Registering " + GetConditionName()

classEnum = stat_level_assassin
classSpecModule = __import__('class021_assassin')

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

def AssassinSneakAttackDice(attachee, args, evt_obj):
	ass_lvl = attachee.stat_level_get(classEnum)
	if evt_obj.data1 == classEnum: #class leveling up
		ass_lvl = ass_lvl + 1 
	evt_obj.return_val += 1+(ass_lvl-1) /2
	return 0

def AssassinPoisonSaveBonus(attachee, args, evt_obj):
	ass_lvl = attachee.stat_level_get(classEnum)
	if ass_lvl < 2:
		return 0
	if evt_obj.flags & 8: # D20STD_F_POISON
		value = ass_lvl / 2
		evt_obj.bonus_list.add(value, 0, 137)
	return 0



classSpecObj = PythonModifier(GetConditionName(), 0)
classSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())
classSpecObj.AddHook(ET_OnD20PythonQuery, "Sneak Attack Dice", AssassinSneakAttackDice, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_NONE, AssassinPoisonSaveBonus, ())


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
		evt_obj.bonus_list.add(-1, 0, 137)  # denotes incomplete spell selection
	return 1


def OnLevelupSpellsFinalize(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	classSpecModule.LevelupSpellsFinalize(attachee)
	return


def AssassinSpellFailure(attachee, args, evt_obj):
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
			
		if attachee.d20_query("Improved Armored Casting") and (armor_flags == ARMOR_TYPE_MEDIUM):
			return 0

	evt_obj.return_val += item.obj_get_int(obj_f_armor_arcane_spell_failure)
	return 0

# spellCasterSpecObj = PythonModifier(GetSpellCasterConditionName(), 8)
# spellCasterSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
classSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
classSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Activate, OnInitLevelupSpellSelection, ())
classSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())
classSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Check_Complete, OnLevelupSpellsCheckComplete, ())
classSpecObj.AddHook(ET_OnD20Query, EK_Q_Get_Arcane_Spell_Failure, AssassinSpellFailure, ())

#############################
#    Hide in Plain Sight    #
#############################

def HideInPlainSightQuery(attachee, args, evt_obj):
	evt_obj.return_val = 1
	return 0

hips_feat = PythonModifier("Hide In Plain Sight Feat", 2)
hips_feat.MapToFeat("Hide in Plain Sight")
hips_feat.AddHook(ET_OnD20PythonQuery, "Can Hide In Plain Sight", HideInPlainSightQuery, () )



##########################
#     Death Attack       #
##########################

deathAttackStudyEnum = 2100
def AssassinDeathAttackRadial(attachee, args, evt_obj):
	radial_action = tpdp.RadialMenuEntryPythonAction("Study Target", D20A_PYTHON_ACTION, deathAttackStudyEnum, 0, "TAG_INTERFACE_HELP")
	spell_data = tpdp.D20SpellData(3210)
	ass_lvl = attachee.stat_level_get(classEnum)
	spell_data.set_spell_level(ass_lvl)
	radial_action.set_spell_data(spell_data)
	radial_action.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
	#print "Death attack radial"
	if attachee.d20_query("Has Studied Target"):
		#print "Has studied target (radial)"
		radial_parent = tpdp.RadialMenuEntryParent("Death Attack")
		#print "Created radial parent"
		par_node_id = radial_parent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
		#print "Added radial parent, ID " + str(par_node_id)
		check_box_paralyze = tpdp.RadialMenuEntryToggle("Paralyze", "TAG_INTERFACE_HELP")
		check_box_paralyze.link_to_args(args, 1)
		check_box_paralyze.add_as_child(attachee, par_node_id)

	return 0


def OnStudyTargetPerform(attachee, args, evt_obj):
	#print "Performing Death Attack - Study Target"
	if evt_obj.d20a.target == OBJ_HANDLE_NULL:
		print "no target! PLS HANDLE ME"
	old_spell_id = args.get_arg(2)
	if old_spell_id:
		spell_packet = tpdp.SpellPacket(old_spell_id)
		prev_tgt = spell_packet.get_target(0)
		if prev_tgt != OBJ_HANDLE_NULL and prev_tgt.d20_query("Is Death Attack Target"):
			prev_tgt.d20_send_signal("Death Attack Target End", old_spell_id)
			prev_tgt.float_text_line("Target removed", tf_white)

	# put the new spell_id in arg2
	new_spell_id = tpactions.get_new_spell_id()
	args.set_arg(2, new_spell_id)
	# register the spell in the spells_cast repository so it triggers the spell scripts (Spell3210 - Death Attack.py)
	cur_seq = tpactions.get_cur_seq()
	tpactions.register_spell_cast(cur_seq.spell_packet, new_spell_id)
	tpactions.trigger_spell_effect(new_spell_id)
	return 0


def DeathAtkResetSneak(attachee, args, evt_obj):
	#print "sneak attack status reset"
	args.set_arg(0, 0)
	return 0


def DeathAtkRegisterSneak(attachee, args, evt_obj):
	#print "sneak attack registered"
	args.set_arg(0, 1)
	return 0


def DeathAtkDamage(attachee, args, evt_obj):
	#print "Death Attack Sneak Attack Not active"
	if not args.get_arg(0):
		return 0
	#print "DeathAtkDamage"
	if evt_obj.attack_packet.get_flags() & D20CAF_RANGED: # Death Attack is for Melee attacks only
		return 0
	tgt = evt_obj.attack_packet.target
	if tgt == OBJ_HANDLE_NULL:
		return 0
	#print "got target"
	spell_id = args.get_arg(2)
	if spell_id == 0:
		return 0
	#print "got spell id"
	spell_packet = tpdp.SpellPacket(spell_id)
	if tgt != spell_packet.get_target(0):
		return 0
	#print "target is spell target"

	if not tgt.d20_query("Is Death Attack Ready"):
		return 0
	#print "DeathAtkDamage target ok"

	# check that target is unaware of assassin (and if it is, override it if it's helpless)
	if (tgt.is_active_combatant() and not tgt.d20_query(Q_Flatfooted)) and tgt.can_see(attachee) and (not tgt.d20_query(Q_Helpless)):
		game.create_history_freeform(tgt.description + " notices " + attachee.description + ", Death Attack unsuccessful...\n\n")
		return 0
	#print "Ending Death Attack Target"
	tgt.d20_send_signal("Death Attack Target End", spell_id) # end the target's Death Attack Target status
	args.set_arg(0,0)

	ass_level = attachee.stat_level_get(classEnum)
	int_level = attachee.stat_level_get(stat_intelligence)
	int_mod = (int_level - 10)/2

	game.create_history_freeform(attachee.description + " attempting Death Attack...\n\n")

	if tgt.saving_throw_spell( 10 + ass_level+ int_mod, D20_Save_Fortitude, D20STD_F_NONE, attachee, spell_id):
		tgt.float_mesfile_line('mes\\spell.mes', 30001)
		game.create_history_freeform("Death Attack failed.\n\n")
	else:
		attachee.float_text_line("Death Attack!")
		if not args.get_arg(1): # death effect
			tgt.critter_kill_by_effect(attachee)
			game.create_history_freeform("Killed by Death Attack!\n\n")
		else: # paralysis effect
			tgt.condition_add_with_args("Paralyzed", ass_level + game.random_range(1,6),0,0)
			game.create_history_freeform("Target paralyzed.\n\n")
	return 0

def HasStudiedTarget(attachee, args, evt_obj):
	#print "Has Studied Target test:"
	spell_id = args.get_arg(2)
	#print "Spell ID: " + str(spell_id)
	if spell_id == 0:
		return 0
	spell_packet = tpdp.SpellPacket(spell_id)
	#print "Spell packet obtained, checking target"
	tgt = spell_packet.get_target(0)
	#print "got target"
	if tgt == OBJ_HANDLE_NULL:
		return 0
	#print "checking is death attack ready"
	if not tgt.d20_query("Is Death Attack Ready"):
		return 0
	#print "   returning ok"
	evt_obj.return_val = 1
	return 0

death_attack_feat = PythonModifier("Death Attack Feat", 3) # arg0 - sneak attack registered;  arg1 - paralyze target;  arg2 - spell_id
death_attack_feat.MapToFeat("Death Attack")
death_attack_feat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, AssassinDeathAttackRadial, ())
death_attack_feat.AddHook(ET_OnD20PythonActionPerform, deathAttackStudyEnum, OnStudyTargetPerform, ())
death_attack_feat.AddHook(ET_OnToHitBonusFromDefenderCondition, EK_NONE, DeathAtkResetSneak, ())
death_attack_feat.AddHook(ET_OnDealingDamage2, EK_NONE, DeathAtkDamage, ())
death_attack_feat.AddHook(ET_OnD20PythonSignal, "Sneak Attack Damage Applied", DeathAtkRegisterSneak, ())
death_attack_feat.AddHook(ET_OnD20PythonQuery, "Has Studied Target", HasStudiedTarget, ())


def IsDeathAtkTarget(attachee, args, evt_obj):
	evt_obj.return_val = 1
	return 0

def IsDeathAtkReady(attachee, args, evt_obj):
	#print "Is death attack ready query answred by " + attachee.description
	evt_obj.return_val = 3 <= args.get_arg(1) <= 6
	return 0

def DeathAtkTargetRound(attachee, args, evt_obj):
	signal_spell_id = evt_obj.data1
	spell_id = args.get_arg(0)
	if spell_id == signal_spell_id:
		args.set_arg(1, args.get_arg(1) + 1)
	return 0

def DeathAtkTargetRoundsStudied(attachee, args, evt_obj):
	signal_spell_id = evt_obj.data1
	spell_id = args.get_arg(0)
	if spell_id == signal_spell_id:
		evt_obj.return_val = args.get_arg(1)
	return 0

def DeathAtkTargetEnd(attachee, args, evt_obj):
	#print "ending death attack target for " + attachee.description
	signal_spell_id = evt_obj.data1
	spell_id = args.get_arg(0)
	#print "signal spell id: " + str(signal_spell_id) + "   spell id: " + str(spell_id)
	if spell_id == signal_spell_id:
		args.condition_remove()
	return 0

def DeathAtkTargetCountdown(attachee, args, evt_obj):
	numRounds = args.get_arg(1)
	roundsToReduce = evt_obj.data1
	#attachee.float_text_line("Ping")
	if numRounds + roundsToReduce <= 6:
		args.set_arg(1, numRounds + roundsToReduce)
		return 0
	#spell_id = args.get_arg(0)
	#attachee.float_text_line("PONG!")
	args.condition_remove()
	return 0


deathAtkTgt = PythonModifier("Death Attack Target", 3, False) # arg0 - spell id, arg1 - number of rounds
deathAtkTgt.AddHook(ET_OnD20PythonQuery, "Is Death Attack Target", IsDeathAtkTarget, ())
deathAtkTgt.AddHook(ET_OnD20PythonQuery, "Is Death Attack Ready", IsDeathAtkReady, ())
deathAtkTgt.AddHook(ET_OnD20PythonSignal, "Target Study Round", DeathAtkTargetRound, ())
deathAtkTgt.AddHook(ET_OnD20PythonQuery, "Death Attack Target Rounds Studied", DeathAtkTargetRoundsStudied, ())
deathAtkTgt.AddHook(ET_OnD20PythonSignal, "Death Attack Target End", DeathAtkTargetEnd, ())
deathAtkTgt.AddHook(ET_OnBeginRound, EK_NONE, DeathAtkTargetCountdown, ())


