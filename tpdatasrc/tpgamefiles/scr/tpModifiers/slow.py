from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from cond_utils import CondPythonModifier

print "Registering non-spell slow condition"

def ReplaceChecks(critter, args, evt_obj):
	# if slow, prefer longer duration
	if evt_obj.is_modifier('sp-Slow'):
		if evt_obj.arg2 > args.get_arg(0):
			args.condition_remove()
		else:
			evt_obj.return_val = 0
		return 0

	if evt_obj.is_modifier('Slow'):
		if evt_obj.arg1 > args.get_arg(0):
			args.condition_remove()
		else:
			evt_obj.return_val = 0
		return 0

	# remove paralysis can cure this condition, possibly with a save
	if not evt_obj.is_modifier('sp-Remove Paralysis'): return 0

	bonus = evt_obj.arg2

	if bonus == 0:
		remove = True
	else:
		bonlist = tpdp.BonusList()
		bonlist.add(4, 0, 'Remove Paralysis')
		dc = args.get_arg(2)
		will = D20_Save_Will
		flags = D20STD_F_NONE
		remove = bonlist.saving_throw(critter, OBJ_HANDLE_NULL, dc, will, flags)
		
		# saving throw result
		critter.float_mesfile_line('mes\\spell.mes', 30001 if remove else 30002)

	if remove: args.condition_remove()

	return 0

def FloatSlow(critter, args, evt_obj):
	# if it won't affect them, don't float the message
	if not critter.d20_query(Q_Critter_Has_Freedom_of_Movement):
		critter.float_mesfile_line('mes\\spell.mes', 20015, 1)

	return 0

def SlowActions(critter, args, evt_obj):
	if critter.d20_query(Q_Critter_Has_Freedom_of_Movement):
		return 0

	if evt_obj.tb_status.hourglass_state >= 2:
		evt_obj.tb_status.hourglass_state = 2
	else:
		evt_obj.tb_status.hourglass_state = 0

	evt_obj.tb_status.flags |= TBSF_Movement

	return 0

def HalveSpeed(critter, args, evt_obj):
	if not critter.d20_query(Q_Critter_Has_Freedom_of_Movement):
		evt_obj.factor *= 0.5

	return 0

def Penalize(critter, args, evt_obj):
	if not critter.d20_query(Q_Critter_Has_Freedom_of_Movement):
		evt_obj.bonus_list.add(-1, 0, 0xad)

	return 0

# arg0 = duration, particle id, dc, spare, spare
slow = CondPythonModifier("Slow", 5)
slow.AddHook(ET_OnConditionAddPre, EK_NONE, ReplaceChecks, ())
slow.AddHook(ET_OnConditionAdd, EK_NONE, FloatSlow, ())
slow.AddHook(ET_OnTurnBasedStatusInit, EK_NONE, SlowActions, ())
slow.AddHook(ET_OnGetMoveSpeedBase, EK_NONE, HalveSpeed, ())
slow.AddHook(ET_OnGetAC, EK_NONE, Penalize, ())
slow.AddHook(ET_OnToHitBonus2, EK_NONE, Penalize, ())
slow.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, Penalize, ())
slow.AddBeginParticles()
slow.AddEndParticles(skip_end = 1)
slow.AddCancelOut(True, 'sp-Haste')

