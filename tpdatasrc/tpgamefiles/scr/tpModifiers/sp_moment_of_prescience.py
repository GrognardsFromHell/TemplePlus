from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
print "Registering sp_moment_of_prescience"

# args: (0-4)
# 0 - spell_id
# 1 - duration
# 2 - enable for ac
# 3 - enable for save
# 4 - enable for attack
# 5 - enable for skil check (disabled for now)
	
def MomentOfPrescienceRadial(attachee, args, evt_obj):
	spell_id = args.get_arg(0)
	spell_packet = tpdp.SpellPacket(spell_id)
	bonus = min(spell_packet.caster_level, 25)
	bonusString = "+" + str(bonus)

	#Add the top level menu
	radial_parent = tpdp.RadialMenuEntryParent("Moment of Prescience")
	MomentOfPrescienceId = radial_parent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)

	#Add a checkbox to turn on and off the various features of the spell
	checkboxACBonus = tpdp.RadialMenuEntryToggle(bonusString + " to AC next attack", "TAG_INTERFACE_HELP")
	checkboxACBonus.link_to_args(args, 2)
	checkboxACBonus.add_as_child(attachee, MomentOfPrescienceId)

	checkboxACBonus = tpdp.RadialMenuEntryToggle(bonusString + " to next save", "TAG_INTERFACE_HELP")
	checkboxACBonus.link_to_args(args, 3)
	checkboxACBonus.add_as_child(attachee, MomentOfPrescienceId)
	
	checkboxACBonus = tpdp.RadialMenuEntryToggle(bonusString + " to next Attack", "TAG_INTERFACE_HELP")
	checkboxACBonus.link_to_args(args, 4)
	checkboxACBonus.add_as_child(attachee, MomentOfPrescienceId)
	
	#checkboxACBonus = tpdp.RadialMenuEntryToggle(bonusString + " to next Skill Check", "TAG_INTERFACE_HELP")
	#checkboxACBonus.link_to_args(args, 5)
	#checkboxACBonus.add_as_child(attachee, MomentOfPrescienceId)
	return 0
	
def MomentOfPrescienceTooltip(attachee, args, evt_obj):
	# Set the tooltip
	evt_obj.append("Moment of Prescience (" + str(args.get_arg(1)) + " rounds)")
	return 0
	
def MomentOfPrescienceEffectTooltip(attachee, args, evt_obj):
	# Set the tooltip
	evt_obj.append(tpdp.hash("MOMENT_OF_PRESCIENCE"), -2, " (" + str(args.get_arg(1)) + " rounds)")
	return 0
	
def MomentOfPrescienceAcBonus(attachee, args, evt_obj):
	#Test to make sure it is not called from the character sheet
	attacker = evt_obj.attack_packet.attacker
	if attacker == OBJ_HANDLE_NULL or attacker == attachee:
		return 0
	
	enabled_flag = args.get_arg(2)
	if enabled_flag:
		spell_id = args.get_arg(0)
		spell_packet = tpdp.SpellPacket(spell_id)
		bonus = min(spell_packet.caster_level, 25)
		evt_obj.bonus_list.add(bonus, 0, "Moment of Prescience")  #  Insight Bonus
		args.remove_spell()
		args.remove_spell_mod()
	return 0
	
def MomentOfPrescienceSaveBonus(attachee, args, evt_obj):
	if not (evt_obj.flags & 1<<(D20STD_F_FINAL_ROLL-1)):
		return 0
	
	enabled_flag = args.get_arg(3)
	if enabled_flag:
		spell_id = args.get_arg(0)
		spell_packet = tpdp.SpellPacket(spell_id)
		bonus = min(spell_packet.caster_level, 25)
		evt_obj.bonus_list.add(bonus, 0, "Moment of Prescience")  #  Insight Bonus
		args.remove_spell()
		args.remove_spell_mod()
	return 0
	
def MomentOfPrescienceAttackBonus(attachee, args, evt_obj):
	#Test to make sure it is not called from the character sheet
	if not (evt_obj.attack_packet.get_flags() & D20CAF_FINAL_ATTACK_ROLL):
		return 0
	
	enabled_flag = args.get_arg(4)
	if enabled_flag:
		spell_id = args.get_arg(0)
		spell_packet = tpdp.SpellPacket(spell_id)
		bonus = min(spell_packet.caster_level, 25)
		evt_obj.bonus_list.add(bonus, 0, "Moment of Prescience")  #  Insight Bonus
		args.remove_spell()
		args.remove_spell_mod()
	return 0
	
#def MomentOfPrescienceSkillCheck(attachee, args, evt_obj):
#	enabled_flag = args.get_arg(5)
#	if enabled_flag:
#		spell_id = args.get_arg(0)
#		spell_packet = tpdp.SpellPacket(spell_id)
#		bonus = min(spell_packet.caster_level, 25)
#		evt_obj.bonus_list.add(bonus, 0, "Moment of Prescience")  #  Insight Bonus
#		args.condition_remove()
#	return 0

def MomentOfPrescienceHasSpellActive(attachee, args, evt_obj):
	evt_obj.return_val = 1
	return 0
	
def MomentOfPrescienceKilled(attachee, args, evt_obj):
	args.remove_spell()
	args.remove_spell_mod()
	return 0
	
def MomentOfPrescienceSpellEnd(attachee, args, evt_obj):
	print "MomentOfPrescienceSpellEnd"
	spell_id = args.get_arg(0)
	if evt_obj.data1 == spell_id:
		game.particles( 'sp-Moment of Prescience-END', attachee)
	return 0

momentOfPrescience = PythonModifier("sp-Moment of Prescience", 6) #
momentOfPrescience.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, MomentOfPrescienceRadial, ())
momentOfPrescience.AddHook(ET_OnGetTooltip, EK_NONE, MomentOfPrescienceTooltip, ())
momentOfPrescience.AddHook(ET_OnGetEffectTooltip, EK_NONE, MomentOfPrescienceEffectTooltip, ())
momentOfPrescience.AddHook(ET_OnGetAC, EK_NONE, MomentOfPrescienceAcBonus, ())
momentOfPrescience.AddHook(ET_OnSaveThrowLevel , EK_NONE , MomentOfPrescienceSaveBonus, ())
momentOfPrescience.AddHook(ET_OnToHitBonus2, EK_NONE, MomentOfPrescienceAttackBonus, ())
#momentOfPrescience.AddHook(ET_OnGetSkillLevel, EK_NONE, MomentOfPrescienceSkillCheck, ())
momentOfPrescience.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, MomentOfPrescienceHasSpellActive, ())
momentOfPrescience.AddHook(ET_OnD20Signal, EK_S_Killed, MomentOfPrescienceKilled, ())
momentOfPrescience.AddHook(ET_OnD20Signal, EK_S_Spell_End, MomentOfPrescienceSpellEnd, ())
momentOfPrescience.AddSpellDispelCheckStandard()
momentOfPrescience.AddSpellTeleportPrepareStandard()
momentOfPrescience.AddSpellTeleportReconnectStandard()
momentOfPrescience.AddSpellCountdownStandardHook()
