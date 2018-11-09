from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Oaken Resilience:   Complete Divine, p. 82

OakenResilienceEnum = 2701

print "Registering Oaken Resilience"

def OakenResilienceRadial(attachee, args, evt_obj):
	isAdded = attachee.condition_add_with_args("Oaken Resilience Effect",0,0) # adds the "Oaken Resilience" condition on first radial menu build
	radialAction = tpdp.RadialMenuEntryPythonAction(-1, D20A_PYTHON_ACTION, OakenResilienceEnum, 0, "TAG_INTERFACE_HELP")
	radialAction.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def OnOakenResilienceEffectCheck(attachee, args, evt_obj):
	#Don't allow if the effect is already enabled
	if args.get_arg(0):
		evt_obj.return_val = AEC_INVALID_ACTION
		return 0
	
	#Get the current number of wild shape charges
	WildShapeCharges = attachee.d20_query("Wild Shape Charges")

	#Check for remaining wild shape attempts
	if (WildShapeCharges < 1):
		evt_obj.return_val = AEC_OUT_OF_CHARGES
		return 0

	return 1

def OnOakenResilienceEffectPerform(attachee, args, evt_obj):
	#Deduct a wild shape charge
	attachee.d20_send_signal("Deduct Wild Shape Charge")
	
	#Set to active
	args.set_arg(0, 1)

	#Lasts 10 minute duration in rounds
	args.set_arg(1, 100)
	
	return 0

def OakenResilienceEffectBeginRound(attachee, args, evt_obj):	
	# not active, nothing more to do
	if not args.get_arg(0): 
		return 0

	#calculate the number of rounds left
	numRounds = args.get_arg(1)
	roundsToReduce = evt_obj.data1
	if numRounds - roundsToReduce > 0:
		args.set_arg(1, numRounds - roundsToReduce ) #decrement the number of rounds
	else:
		args.set_arg(0, 0) # set effect to inactive
	return 0

def OakenResilienceEffectTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0
		
	evt_obj.append("Oaken Resilience")

	return 0

def OakenResilienceEffectTooltipEffect(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0

	# Set the tooltip
	evt_obj.append(tpdp.hash("OAKEN_RESILIENCE"), -2, " (" + str(args.get_arg(1)) + " rounds)")
	return 0
	
def OakenResiliencePoisonImmunity(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0
		
	evt_obj.return_val = 1
	return 0
	
def OakenResilienceCriticalImmunity(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0
		
	evt_obj.return_val = 1
	return 0
	
def OakenResilienceAddPre(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0

	IsSleep = evt_obj.is_modifier("sp-Sleep")
	if IsSleep:
		evt_obj.return_val = 0
		attachee.float_mesfile_line( 'mes\\combat.mes', 5059, tf_red ) # "Sleep Immunity"
		game.create_history_from_pattern(31, attachee, OBJ_HANDLE_NULL)
		return 0
		
	IsParalyzed = evt_obj.is_modifier("Paralyzed")
	if IsParalyzed:
		evt_obj.return_val = 0
		attachee.float_text_line( "Paralysis Immunity", tf_red )
		return 0
		
	IsStunned = evt_obj.is_modifier("Stunned")
	if IsStunned:
		evt_obj.return_val = 0
		attachee.float_text_line( "Stun Immunity", tf_red )
		return 0
		
	#Don't know of any polymorph effects for providing immunity
		
	return 0

	
def OakenResilienceTripDefenseBonus(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0
	
	#Check that this is a trip before applying the bonus
	if (evt_obj.flags & 1 and evt_obj.flags & 2):
		evt_obj.bonus_list.add(8, 0, "Oaken Resilience") # untyped
	
	return 0

#Setup the feat
OakenResilienceFeat = PythonModifier("Oaken Resilience Feat", 2) # spare, spare
OakenResilienceFeat.MapToFeat("Oaken Resilience")
OakenResilienceFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, OakenResilienceRadial, ())

#Setup the effect
OakenResilienceEffect = PythonModifier("Oaken Resilience Effect", 4) #active, rounds left, spare, spare
OakenResilienceEffect.AddHook(ET_OnBeginRound, EK_NONE, OakenResilienceEffectBeginRound, ())
OakenResilienceEffect.AddHook(ET_OnD20PythonActionCheck, OakenResilienceEnum, OnOakenResilienceEffectCheck, ())
OakenResilienceEffect.AddHook(ET_OnD20PythonActionPerform, OakenResilienceEnum, OnOakenResilienceEffectPerform, ())
OakenResilienceEffect.AddHook(ET_OnGetTooltip, EK_NONE, OakenResilienceEffectTooltip, ())
OakenResilienceEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, OakenResilienceEffectTooltipEffect, ())
OakenResilienceEffect.AddHook(ET_OnD20Query, EK_Q_Critter_Is_Immune_Critical_Hits, OakenResilienceCriticalImmunity, ())
OakenResilienceEffect.AddHook(ET_OnD20Query, EK_Q_Critter_Is_Immune_Poison, OakenResiliencePoisonImmunity, ())
OakenResilienceEffect.AddHook(ET_OnConditionAddPre, EK_NONE, OakenResilienceAddPre, ())
OakenResilienceEffect.AddHook(ET_OnGetAbilityCheckModifier, EK_STAT_DEXTERITY, OakenResilienceTripDefenseBonus, ())
OakenResilienceEffect.AddHook(ET_OnGetAbilityCheckModifier, EK_STAT_STRENGTH, OakenResilienceTripDefenseBonus, ())

