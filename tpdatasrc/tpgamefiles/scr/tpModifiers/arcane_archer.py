from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils
import tpactions

###################################################

def GetConditionName():
	return "Arcane Archer"

#def GetSpellCasterConditionName():
#	return "Arcane Archer Spellcasting"

print "Registering " + GetConditionName()

classEnum = stat_level_arcane_archer
imbueArrowEnum = 2000
seekerArrowEnum = 2001
phaseArrowEnum = 2002
hailOfArrowsEnum = 2003
deathArrowEnum = 2004
###################################################

tpdp.register_bard_song_stopping_python_action(imbueArrowEnum)

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


def OnGetBaseCasterLevel(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	classLvl = attachee.stat_level_get(classEnum)
	evt_obj.bonus_list.add(classLvl, 0, 137)
	return 0

classSpecObj = PythonModifier(GetConditionName(), 0)
classSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())
classSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())


# Helper Functions
def is_ranged_weapon(weap):
	if weap == OBJ_HANDLE_NULL:
		return 0
	weap_flags = weap.obj_get_int(obj_f_weapon_flags)
	if not (weap_flags & OWF_RANGED_WEAPON):
		return 0
	return 1

def has_ranged_weapon(attachee):
	weap = attachee.item_worn_at(3)
	return is_ranged_weapon(weap)

### Enhance Arrow

def EnhanceArrowDamage(attachee, args, evt_obj):
	classLvl = attachee.stat_level_get(classEnum)
	bon_lvl = 1 + (classLvl - 1) / 2
	weapon_used = evt_obj.attack_packet.get_weapon_used()
	if is_ranged_weapon(weapon_used):
		evt_obj.damage_packet.bonus_list.add_from_feat(bon_lvl, 12, 147, "Enhance Arrow")
		evt_obj.damage_packet.attack_power |= D20DAP_MAGIC
	return 0


def EnhanceArrowToHit(attachee, args, evt_obj):
	classLvl = attachee.stat_level_get(classEnum)
	bon_lvl = 1 + (classLvl - 1) / 2
	weapon_used = evt_obj.attack_packet.get_weapon_used()
	if is_ranged_weapon(weapon_used):
		evt_obj.bonus_list.add_from_feat(bon_lvl, 12, 147, "Enhance Arrow")
	return 0


classSpecObj.AddHook(ET_OnDealingDamage, EK_NONE, EnhanceArrowDamage, ())
classSpecObj.AddHook(ET_OnToHitBonus2, EK_NONE, EnhanceArrowToHit, ())



### Imbue Arrow feat

def ImbueOk(attachee, spData):
	return (spData.is_area_spell() or spData.is_mode_target(MODE_TARGET_CONE)) and attachee.can_cast_spell(spData)
	
def HasSlotsRemaining(attachee, knSp):
	classCode = knSp.spell_class
	spellLevel = knSp.spell_level
	max = attachee.get_num_spells_per_day(classCode, spellLevel)
	used = attachee.get_num_spells_used(classCode, spellLevel)
	return used < max

def ImbueArrowRadial(attachee, args, evt_obj):

	weap = attachee.item_worn_at(3)
	if not is_ranged_weapon(weap):
		weap = attachee.item_worn_at(4)
		if not is_ranged_weapon(weap):
			return 0

	known_spells = attachee.spells_known

	radial_parent = tpdp.RadialMenuEntryParent("Imbue Arrow")
	imb_arrow_id = radial_parent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)

	# create the spell level nodes
	spell_level_ids = []
	for p in range(0,10):
		spell_level_node = tpdp.RadialMenuEntryParent(str(p))
		spell_level_ids.append( spell_level_node.add_as_child(attachee, imb_arrow_id) )

	for knSp in known_spells:
		if knSp.is_naturally_cast() and ImbueOk(attachee, knSp) and attachee.spontaneous_spells_remaining(knSp.spell_class, knSp.spell_level):
			spell_node = tpdp.RadialMenuEntryPythonAction(knSp, D20A_PYTHON_ACTION, imbueArrowEnum,0)
			spell_node.add_as_child(attachee, spell_level_ids[knSp.spell_level])

	mem_spells = attachee.spells_memorized
	for memSp in mem_spells:
		if (not memSp.is_used_up()) and ImbueOk(attachee, memSp):
			spell_node = tpdp.RadialMenuEntryPythonAction(memSp, D20A_PYTHON_ACTION, imbueArrowEnum, 0)
			spell_node.add_as_child(attachee, spell_level_ids[memSp.spell_level])
	return 0

def ImbueArrowPerform(attachee, args, evt_obj):
	cur_seq = tpactions.get_cur_seq()
	tgtLoc = cur_seq.spell_packet.loc.loc_and_offsets
	arrowTgt = evt_obj.d20a.target
	min_dist = 10000
	if arrowTgt == attachee: # e.g. when player brings up radial menu by right clicking the character
		arrowTgt = OBJ_HANDLE_NULL
	if arrowTgt == OBJ_HANDLE_NULL:
		for p in range(0,32):
			tgt = cur_seq.spell_packet.get_target(p)
			if tgt == OBJ_HANDLE_NULL:
				break
			dist_to_tgt = tgt.distance_to(tgtLoc.get_location(), tgtLoc.off_x, tgtLoc.off_y)
			#print "Distance to tgt " + str(tgt) + ": " + str(dist_to_tgt)
			if arrowTgt == OBJ_HANDLE_NULL or dist_to_tgt < min_dist:
				if dist_to_tgt < 1 or (not tgt.is_friendly(attachee) ):
					min_dist = dist_to_tgt
					arrowTgt = tgt
	evt_obj.d20a.target = arrowTgt
	#print "Imbue arrow: target is " + str(arrowTgt)

	# roll to hit
	evt_obj.d20a.flags |= D20CAF_RANGED
	evt_obj.d20a.to_hit_processing()
	isCritical = 0
	if evt_obj.d20a.flags & D20CAF_CRITICAL:
		isCritical = 1

	#print "Imbue arrow: setting new spell ID"
	evt_obj.d20a.filter_spell_targets(cur_seq.spell_packet)
	new_spell_id = tpactions.get_new_spell_id()
	tpactions.register_spell_cast(cur_seq.spell_packet, new_spell_id)
	evt_obj.d20a.spell_id = new_spell_id
	cur_seq.spell_action.spell_id = new_spell_id
	cur_seq.spell_packet.debit_spell()
	#print "Imbue arrow: spell ID registered " + str(new_spell_id)




	# provoke hostility if applicable
	for p in range(0, 32):
		tgt = cur_seq.spell_packet.get_target(p)
		if tgt == OBJ_HANDLE_NULL:
			break
		if game.is_spell_harmful(cur_seq.spell_packet.spell_enum, attachee, tgt):
			attachee.attack(tgt)

	attachee.d20_send_signal(S_Spell_Cast, new_spell_id)
	for p in range(0, 32):
		tgt = cur_seq.spell_packet.get_target(p)
		if tgt == OBJ_HANDLE_NULL:
			break
		tgt.d20_send_signal(S_Spell_Cast, new_spell_id)

	if attachee.anim_goal_push_attack(arrowTgt, game.random_range(0,2), isCritical ,0):
		new_anim_id = attachee.anim_goal_get_new_id()
		evt_obj.d20a.flags |= D20CAF_NEED_ANIM_COMPLETED
		evt_obj.d20a.anim_id = new_anim_id

	return 0

def ImbueArrowActionFrame(attachee, args, evt_obj):
	#print "Imbue Arrow Action Frame"
	cur_seq = tpactions.get_cur_seq()

	tgt = evt_obj.d20a.target
	#print "Imbue Arrow Target: " + str(tgt)
	wpn = attachee.item_worn_at(item_wear_weapon_primary)
	if tgt != OBJ_HANDLE_NULL and wpn != OBJ_HANDLE_NULL:
		projectileProto = wpn.get_weapon_projectile_proto()
		tgtLoc = cur_seq.spell_packet.loc
		projectileHandle = evt_obj.d20a.create_projectile_and_throw(projectileProto, tgtLoc.loc_and_offsets)
		projectileHandle.obj_set_float(obj_f_offset_z, 60.0)
		cur_seq.spell_packet.spell_id = evt_obj.d20a.spell_id
		cur_seq.spell_packet.set_projectile(0, projectileHandle)
		ammoItem = OBJ_HANDLE_NULL
		if evt_obj.d20a.projectile_append(projectileHandle, ammoItem):
			#print "Imbue Arrow Action Frame: Projectile Appended"
			attachee.apply_projectile_particles(projectileHandle, evt_obj.d20a.flags)
			evt_obj.d20a.flags |= D20CAF_NEED_PROJECTILE_HIT
	else:
		tpactions.trigger_spell_effect(evt_obj.d20a.spell_id)
	return 0

imbArrowFeat = PythonModifier("Imbue Arrow Feat", 3)
imbArrowFeat.MapToFeat("Imbue Arrow")
imbArrowFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, ImbueArrowRadial, ())
imbArrowFeat.AddHook(ET_OnD20PythonActionPerform, imbueArrowEnum, ImbueArrowPerform, ())
imbArrowFeat.AddHook(ET_OnD20PythonActionFrame, imbueArrowEnum, ImbueArrowActionFrame, ())

### Seeker Arrow feat

def SeekerArrowRadial(attachee, args, evt_obj):
	if not has_ranged_weapon(attachee):
		return 0

	if args.get_arg(0):
		print str(args.get_arg(0))
		return 0

	radial_action = tpdp.RadialMenuEntryPythonAction("Seeker Arrow",D20A_PYTHON_ACTION, seekerArrowEnum, 0, "TAG_INTERFACE_HELP")
	radial_action.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)

	return 0

def SeekerArrowPerform(attachee, args, evt_obj):
	cur_seq = tpactions.get_cur_seq()
	arrowTgt = evt_obj.d20a.target
	min_dist = 10000
	#print "Seeker arrow: target is " + str(arrowTgt)

	# roll to hit
	# cover bonus won't be applied because it doesn't appear in the ActionCheck callback
	# TODO: apply a negator condition for the concealment chances
	evt_obj.d20a.flags |= D20CAF_RANGED
	evt_obj.d20a.to_hit_processing()
	isCritical = 0
	if evt_obj.d20a.flags & D20CAF_CRITICAL:
		isCritical = 1


	if attachee.anim_goal_push_attack(arrowTgt, game.random_range(0,2), isCritical ,0):
		new_anim_id = attachee.anim_goal_get_new_id()
		#print "new anim id: " + str(new_anim_id)
		evt_obj.d20a.flags |= D20CAF_NEED_ANIM_COMPLETED
		evt_obj.d20a.anim_id = new_anim_id

	return 0

def SeekerArrowActionFrame(attachee, args, evt_obj):
	#print "Seeker Arrow Action Frame"
	args.set_arg(0, 1) # mark as used this day

	tgt = evt_obj.d20a.target
	#print "Seeker Arrow Target: " + str(tgt)
	wpn = attachee.item_worn_at(item_wear_weapon_primary)
	if tgt != OBJ_HANDLE_NULL and wpn != OBJ_HANDLE_NULL:
		projectileProto = wpn.get_weapon_projectile_proto()
		projectileHandle = evt_obj.d20a.create_projectile_and_throw(projectileProto, tgt)
		projectileHandle.obj_set_float(obj_f_offset_z, 60.0)
		ammoItem = OBJ_HANDLE_NULL
		if evt_obj.d20a.projectile_append(projectileHandle, ammoItem):
			#print "Seeker Arrow Action Frame: Projectile Appended"
			attachee.apply_projectile_particles(projectileHandle, evt_obj.d20a.flags)
			evt_obj.d20a.flags |= D20CAF_NEED_PROJECTILE_HIT
	return 0

def SeekerArrowReset(attachee, args, evt_obj):
	args.set_arg(0, 0)

seekerArrowFeat = PythonModifier("Seeker Arrow Feat", 3) # arg0 - used this day
seekerArrowFeat.MapToFeat("Seeker Arrow")
seekerArrowFeat.AddHook(ET_OnConditionAdd, EK_NONE, SeekerArrowReset, ())
seekerArrowFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, SeekerArrowRadial, ())
seekerArrowFeat.AddHook(ET_OnD20PythonActionPerform, seekerArrowEnum, SeekerArrowPerform, ())
seekerArrowFeat.AddHook(ET_OnD20PythonActionFrame, seekerArrowEnum, SeekerArrowActionFrame, ())
seekerArrowFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, SeekerArrowReset, ())


####################
### Phase Arrow feat

def PhaseArrowRadial(attachee, args, evt_obj):
	if not has_ranged_weapon(attachee):
		return 0

	if args.get_arg(0):
		return 0

	radial_action = tpdp.RadialMenuEntryPythonAction("Phase Arrow",D20A_PYTHON_ACTION, phaseArrowEnum, 0, "TAG_INTERFACE_HELP")
	radial_action.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)

	return 0

def PhaseArrowPerform(attachee, args, evt_obj):
	cur_seq = tpactions.get_cur_seq()
	arrowTgt = evt_obj.d20a.target
	min_dist = 10000
	#print "Phase arrow: target is " + str(arrowTgt)

	# roll to hit
	evt_obj.d20a.flags |= D20CAF_RANGED
	evt_obj.d20a.to_hit_processing()
	isCritical = 0
	if evt_obj.d20a.flags & D20CAF_CRITICAL:
		isCritical = 1


	if attachee.anim_goal_push_attack(arrowTgt, game.random_range(0,2), isCritical ,0):
		new_anim_id = attachee.anim_goal_get_new_id()
		#print "new anim id: " + str(new_anim_id)
		evt_obj.d20a.flags |= D20CAF_NEED_ANIM_COMPLETED
		evt_obj.d20a.anim_id = new_anim_id

	return 0

def PhaseArrowActionFrame(attachee, args, evt_obj):
	#print "Phase Arrow Action Frame"
	args.set_arg(0, 1) # mark as used this day

	tgt = evt_obj.d20a.target
	#print "Phase Arrow Target: " + str(tgt)
	wpn = attachee.item_worn_at(item_wear_weapon_primary)
	if tgt != OBJ_HANDLE_NULL and wpn != OBJ_HANDLE_NULL:
		projectileProto = wpn.get_weapon_projectile_proto()
		projectileHandle = evt_obj.d20a.create_projectile_and_throw(projectileProto, tgt)
		projectileHandle.obj_set_float(obj_f_offset_z, 60.0)
		ammoItem = OBJ_HANDLE_NULL
		if evt_obj.d20a.projectile_append(projectileHandle, ammoItem):
			#print "Phase Arrow Action Frame: Projectile Appended"
			attachee.apply_projectile_particles(projectileHandle, evt_obj.d20a.flags)
			evt_obj.d20a.flags |= D20CAF_NEED_PROJECTILE_HIT
	return 0

def PhaseArrowReset(attachee, args, evt_obj):
	args.set_arg(0, 0)

def PhaseArrowArmorNullifier(attachee, args, evt_obj):
	if evt_obj.attack_packet.action_type != tpdp.D20ActionType.PythonAction:
		return 0
	if evt_obj.attack_packet.event_key != phaseArrowEnum:
		return 0
	armor_ac_bonus_type = 28
	shield_ac_bonus_type = 29
	armor_enh_bonus_type = 12
	shield_enh_bonus_type = 33
	evt_obj.bonus_list.add_cap(armor_ac_bonus_type, 0, 114, "Phase Arrow")
	evt_obj.bonus_list.add_cap(shield_ac_bonus_type, 0, 114, "Phase Arrow")
	evt_obj.bonus_list.add_cap(armor_enh_bonus_type, 0, 114, "Phase Arrow")
	evt_obj.bonus_list.add_cap(shield_enh_bonus_type, 0, 114, "Phase Arrow")
	return 0

phaseArrowFeat = PythonModifier("Phase Arrow Feat", 3) # arg0 - used this day
phaseArrowFeat.MapToFeat("Phase Arrow")
phaseArrowFeat.AddHook(ET_OnConditionAdd, EK_NONE, PhaseArrowReset, ())
phaseArrowFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, PhaseArrowRadial, ())
phaseArrowFeat.AddHook(ET_OnD20PythonActionPerform, phaseArrowEnum, PhaseArrowPerform, ())
phaseArrowFeat.AddHook(ET_OnD20PythonActionFrame, phaseArrowEnum, PhaseArrowActionFrame, ())
phaseArrowFeat.AddHook(ET_OnGetAcModifierFromAttacker, EK_NONE, PhaseArrowArmorNullifier, ())
phaseArrowFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, PhaseArrowReset, ())


#######################
### Hail of Arrows feat

def HailOfArrowsRadial(attachee, args, evt_obj):
	if not has_ranged_weapon(attachee):
		return 0
	if args.get_arg(0):
		return 0

	radial_action = tpdp.RadialMenuEntryPythonAction("Hail of Arrows",D20A_PYTHON_ACTION, hailOfArrowsEnum, 0, "TAG_INTERFACE_HELP")
	spell_data = tpdp.D20SpellData(3180)
	aarc_lvl = attachee.stat_level_get(classEnum)
	spell_data.set_spell_class(classEnum)
	spell_data.set_spell_level(aarc_lvl)
	radial_action.set_spell_data(spell_data)
	radial_action.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)

	return 0

def HailOfArrowsReset(attachee, args, evt_obj):
	args.set_arg(0, 0)


hailOfArrowsFeat = PythonModifier("Hail of Arrows Feat", 3) # arg0 - used this day
hailOfArrowsFeat.MapToFeat("Hail of Arrows")
hailOfArrowsFeat.AddHook(ET_OnConditionAdd, EK_NONE, HailOfArrowsReset, ())
hailOfArrowsFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, HailOfArrowsReset, ())
hailOfArrowsFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, HailOfArrowsRadial, ())



#######################
### Death Arrow feat

def DeathArrowRadial(attachee, args, evt_obj):
	if not has_ranged_weapon(attachee):
		return 0
	if args.get_arg(0):
		return 0
	radial_action = tpdp.RadialMenuEntryPythonAction("Arrow of Death",D20A_PYTHON_ACTION, deathArrowEnum, 0, "TAG_INTERFACE_HELP")
	radial_action.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)

	return 0

def DeathArrowReset(attachee, args, evt_obj):
	args.set_arg(0, 0)
	args.set_arg(1, 0)
	args.set_arg(2, 0)
	return 0


def DeathArrowDamage(attachee, args, evt_obj):
	#print "Death Arrow Damage"
	if not args.get_arg(2): # not anticipating death attack
		return 0
	if (evt_obj.attack_packet.get_flags() & D20CAF_RANGED) == 0:
		return 0
	tgt = evt_obj.attack_packet.target
	if tgt == OBJ_HANDLE_NULL:
		return 0

	game.create_history_freeform(tgt.description + " hit by Arrow of Death...\n\n")

	if tgt.saving_throw( 20, D20_Save_Fortitude, D20STD_F_NONE, attachee):
		tgt.float_mesfile_line('mes\\spell.mes', 30001)
		game.create_history_freeform("Death effect failed.\n\n")
	else:
		attachee.float_text_line("Arrow of Death!")
		tgt.critter_kill_by_effect(attachee)
		game.create_history_freeform("Killed by Arrow of Death!\n\n")
	return 0


def IsActive(args):
	if args.get_arg(0):
		return 1
	return 0

def DeathAttackDisable(attachee, args, evt_obj):
	args.set_arg(2, 0) # unset
	return 0

def DeathArrowAttackRollMade(attachee, args, evt_obj):
	if (evt_obj.attack_packet.get_flags() & D20CAF_RANGED) == 0:
		return 0
	if IsActive(args):
		args.set_arg(0, 0)
		args.set_arg(2, 1)
	return 0



def DeathArrowCheck(attachee, args, evt_obj):
	if IsActive(args):
		evt_obj.return_val = AEC_INVALID_ACTION
		return 0
	# check if enough usages / day left
	maxNumPerDay = 1
	if args.get_arg(1) >= maxNumPerDay:
		evt_obj.return_val = AEC_OUT_OF_CHARGES
	return 0

def ODeathArrowPerform(attachee, args, evt_obj):

	if IsActive(args):
		print "Not performing arrow of death ebcause it's already active"
		return 0
	args.set_arg(0, 1) # set to active
	args.set_arg(1, args.get_arg(1) + 1) #increment number used / day
	args.set_arg(2, 0)  # reset expecting damage state
	attachee.float_text_line("Death Arrow equipped", tf_red)
	return 0


deathArrowFeat = PythonModifier("Death Arrow Feat", 3) # arg0 - is active; arg1 - times spent; arg2 - anticipate death attack
deathArrowFeat.MapToFeat("Arrow of Death")
deathArrowFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, DeathArrowRadial, ())
deathArrowFeat.AddHook(ET_OnConditionAdd, EK_NONE, DeathArrowReset, ())
deathArrowFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, DeathArrowReset, ())
deathArrowFeat.AddHook(ET_OnDealingDamage2, EK_NONE, DeathArrowDamage, ())
deathArrowFeat.AddHook(ET_OnGetAcModifierFromAttacker, EK_NONE, DeathArrowAttackRollMade, ()) # signifies that a to hit roll was made
deathArrowFeat.AddHook(ET_OnD20PythonActionCheck, deathArrowEnum, DeathArrowCheck, ())
deathArrowFeat.AddHook(ET_OnD20PythonActionPerform, deathArrowEnum, ODeathArrowPerform, ())
deathArrowFeat.AddHook(ET_OnD20Signal, EK_S_Attack_Made, DeathAttackDisable, ()) # gets triggered at the end of the damage calculation