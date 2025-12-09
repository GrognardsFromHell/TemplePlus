from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering standalone deafness condition"

def CheckRemove(critter, args, evt_obj):
	if evt_obj.is_modifier('sp-Heal'):
		args.condition_remove()
	if evt_obj.is_modifier('sp-Remove Deafness'):
		args.condition_remove()

	return 0

def SetTrue(critter, args, evt_obj):
	evt_obj.return_val = 1

	return 0

def FloatDeafness(critter, args, evt_obj):
	critter.float_mesfile_line('mes\\spell.mes', 20020, 1)
	return 0

def InitiativePenalty(critter, args, evt_obj):
	evt_obj.bonus_list.add(-4, 42, 190)
	return 0

def Tooltip(critter, args, evt_obj):
	evt_obj.append_distinct('Deaf!')
	return 0

def EffTooltip(critter, args, evt_obj):
	evt_obj.append(tpdp.hash('DEAFNESS'), -2, "")
	return 0

def Countdown(critter, args, evt_obj):
	# arg1 non-zero means permanent
	if args.get_arg(1): return 0

	remain = args.get_arg(0) - evt_obj.data1
	if remain < 0:
		args.condition_remove()
	else:
		args.set_arg(0, remain)

	return 0

def Remove(critter, args, evt_obj):
	args.condition_remove()
	return 0

# standalone deafness condition
# arg0 = duration
# arg1 = is permanent
# arg2, arg3 = spare
deafness = PythonModifier("Deafness", 4, 0)
deafness.AddHook(ET_OnConditionAddPre, EK_NONE, CheckRemove, ())
deafness.AddHook(ET_OnConditionAdd, EK_NONE, FloatDeafness, ())
deafness.AddHook(ET_OnD20Query, EK_Q_Critter_Is_Deafened, SetTrue, ())
deafness.AddHook(ET_OnGetInitiativeMod, EK_NONE, InitiativePenalty, ())
deafness.AddHook(ET_OnGetTooltip, EK_NONE, Tooltip, ())
deafness.AddHook(ET_OnGetEffectTooltip, EK_NONE, EffTooltip, ())
deafness.AddHook(ET_OnBeginRound, EK_NONE, Countdown, ())
deafness.AddHook(ET_OnD20Signal, EK_S_Killed, Remove, ())
