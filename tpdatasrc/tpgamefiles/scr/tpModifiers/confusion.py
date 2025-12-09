from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from cond_utils import CondPythonModifier

print "Registering standalone confusion condition"

# Confusion AI state numbers from the existing logic
CONF_NORMAL = 0
CONF_FLEE_CASTER = 5
CONF_DO_NOTHING = 6
CONF_FLEE_RANDOM_OPPONENT = 7
CONF_TARGET_RANDOM_NEARBY = 8
CONF_TARGET_CLOSEST = 9
CONF_RANDOM = 11
CONF_TARGET_SPECIFIC = 13
CONF_RETALIATE = 100

def RenderState(state):
	if state == CONF_NORMAL:
		return "normal action"
	if state == CONF_FLEE_CASTER:
		return "flee caster"
	elif state == CONF_DO_NOTHING:
		return "do nothing"
	elif state == CONF_FLEE_RANDOM_OPPONENT:
		return "flee random opponent"
	elif state == CONF_TARGET_RANDOM_NEARBY:
		return "target random"
	elif state == CONF_TARGET_CLOSEST:
		return "target closest"
	elif state == CONF_RANDOM:
		return "random"
	elif state == CONF_TARGET_SPECIFIC:
		return "attack specific"
	elif state == CONF_RETALIATE:
		return "retaliate"
	else:
		return "{}".format(state)

def ClearState(critter, args, evt_obj):
	args.set_arg(2, 0)
	args.set_args_from_obj(3, OBJ_HANDLE_NULL)

	return 0

# Cancels fleeing AI, for when behavior should switch to something else.
def CancelAIState(critter, args, evt_obj):
	if args.get_arg(2) in [5, 7]:
		critter.ai_stop_fleeing()

def CheckHostility(critter, args, evt_obj):
	action = evt_obj.get_d20_action()

	if not action.is_harmful(): return 0

	CancelAIState(critter, args, evt_obj)

	# 'any confused character who is attacked automatically attacks its attacker
	# on the next round'
	args.set_arg(2, CONF_RETALIATE)
	args.set_args_from_obj(3, action.performer)

	return 0

def CheckState(critter, args, evt_obj):
	confusion_state = args.get_arg(2)

	if evt_obj.return_val == 0 and confusion_state != 0:
		evt_obj.return_val = 1
		evt_obj.data1 = confusion_state

	return 0

def IsConfused(critter, args, evt_obj):
	confusion_state = args.get_arg(2)
	if confusion_state > 0:
		evt_obj.return_val = 1
		if confusion_state in [CONF_TARGET_SPECIFIC, CONF_RETALIATE]:
			evt_obj.set_args_from_obj(args.get_obj_from_args(3))

	return 0

def MatchTarget(critter, args, evt_obj):
	opportunity = evt_obj.get_obj_from_args()
	target = args.get_obj_from_args(3)

	if target == opportunity:
		evt_obj.return_val = 1

	return 0

def RandomAction(critter, args, evt_obj):
	confusion_state = args.get_arg(2) # previous state

	# if the previous state was fleeing, unset flee state in ai
	if confusion_state in [5,7]:
		critter.ai_stop_fleeing()

	# Randomize
	#
	# Official rules say to roll a d100, but all outcomes are multiples of 10%
	# so ...
	roll = dice_new('1d10').roll()
	if confusion_state == CONF_RETALIATE: # been attacked since last turn
		confusion_state = CONF_TARGET_SPECIFIC
	elif roll == 1: # 10% attack caster
		# not implemented, as this effect doesn't have a "caster"
		# attack random instead
		confusion_state = CONF_TARGET_CLOSEST
	elif roll == 2: # 10% act normally
		confusion_state = CONF_NORMAL
	elif roll <= 5: # 30% do nothing
		confusion_state = CONF_DO_NOTHING
	elif roll <= 7: # 20% flee
		confusion_state = CONF_FLEE_RANDOM_OPPONENT
	else: # 30% attack nearest
		confusion_state = CONF_TARGET_CLOSEST

	if confusion_state > 0:
		critter.float_mesfile_line('mes\\spell.mes', 20038, tf_red)

	args.set_arg(2, confusion_state)
	if confusion_state != CONF_TARGET_SPECIFIC:
		# forget about previous target if we aren't retaliating
		args.set_args_from_obj(3, OBJ_HANDLE_NULL)

	critter.ai_process()

	return 0

# arg0 = duration
# arg1 = spare
# arg2 = confusion state
# arg3-4 = hostile target
# arg5-6 = spare
confusion = CondPythonModifier("Confusion", 7)
confusion.AddRemovedBy('sp-Calm Emotions', 'sp-Heal', 'sp-Greater Restoration')
confusion.AddHook(ET_OnD20Signal, EK_S_Action_Recipient, CheckHostility, ())
confusion.AddHook(ET_OnD20Query, EK_Q_AI_Has_Spell_Override, CheckState, ())
confusion.AddHook(ET_OnD20Query, EK_Q_Critter_Is_Confused, IsConfused, ())
confusion.AddHook(ET_OnD20Query, EK_Q_AOOPossible, MatchTarget, ())
confusion.AddHook(ET_OnD20Query, EK_Q_AOOWillTake, MatchTarget, ())
confusion.AddHook(ET_OnConditionAdd, EK_NONE, ClearState, ())
confusion.AddHook(ET_OnConditionRemove, EK_NONE, CancelAIState, ())
confusion.AddHook(ET_OnTurnBasedStatusInit, EK_NONE, RandomAction, ())
