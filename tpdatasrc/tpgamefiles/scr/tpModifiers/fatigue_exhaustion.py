from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
print "Registering fatigue_exhaust"
	
# Note:  Not current disabling run when fatigued.  Charge will be correctly disabled.
	
def FatigueOnAdd(attachee, args, evt_obj):
	partSys = game.particles("Barbarian Fatigue", attachee )
	exhausted = args.get_arg(2)
	if exhausted != 0:
		attachee.float_text_line("Exhausted")
	else:
		attachee.float_text_line("Fatigued")
	args.set_arg(4, partSys)
	return 0
	
def FatigueTooltip(attachee, args, evt_obj):
	# Set the tooltip
	exhausted = args.get_arg(2)
	if exhausted != 0:
		if exhausted == -1:
			evt_obj.append("Exhausted")
		else:
			evt_obj.append("Exhausted (" + str(exhausted) + " rounds)")
	else:
		fatigued = args.get_arg(1)
		if fatigued == -1:
			evt_obj.append("Fatigue")
		else:
			evt_obj.append("Fatigue (" + str(fatigued) + " rounds)")
	return 0
	
def FatigueEffectTooltip(attachee, args, evt_obj):
	# Set the tooltip
	exhausted = args.get_arg(2)
	if exhausted != 0:
		if exhausted == -1:
			evt_obj.append(tpdp.hash("FATIGUE_EXHAUST"), -2, "Exhausted")
		else:
			evt_obj.append(tpdp.hash("FATIGUE_EXHAUST"), -2, "Exhausted (" + str(exhausted) + " rounds)")
	else:
		fatigued = args.get_arg(1)
		if fatigued == -1:
			evt_obj.append(tpdp.hash("FATIGUE_EXHAUST"), -2, "Fatigue")
		else:
			evt_obj.append(tpdp.hash("FATIGUE_EXHAUST"), -2, "Fatigue (" + str(fatigued) + " rounds)")
	return 0

def FatigueDexMod(attachee, args, evt_obj):
	exhaustDuration = args.get_arg(2)
	if exhaustDuration != 0:
		evt_obj.bonus_list.add(-6, 0, "Exhausted")
	else:
		evt_obj.bonus_list.add(-2, 0, "Fatigue")
	return 0
	
def FatigueStrMod(attachee, args, evt_obj):
	exhaustDuration = args.get_arg(2)
	if exhaustDuration != 0:
		evt_obj.bonus_list.add(-6, 0, "Exhausted")
	else:
		evt_obj.bonus_list.add(-2, 0, "Fatigue")
	return 0
	
def ExhaustedMovement(attachee, args, evt_obj):
	#Half movement if exhausted
	exhausted = args.get_arg(2)
	if exhausted != 0:
		evt_obj.factor = evt_obj.factor * .5
	return 0
	
def FatigueBeginRound(attachee, args, evt_obj):
	roundsToReduce = evt_obj.data1
	rageFatigueDuration = args.get_arg(0)
	fatigueDuration = args.get_arg(1)
	exhaustDuration = args.get_arg(2)

	if rageFatigueDuration > 0:
		rageFatigueDuration = rageFatigueDuration - roundsToReduce
		rageFatigueDuration = max(rageFatigueDuration, 0)
		args.set_arg(0, rageFatigueDuration)
		
	
	if exhaustDuration > 0:
		exhaustDuration = exhaustDuration - roundsToReduce
		exhaustDuration = max(exhaustDuration, 0)
		args.set_arg(2, exhaustDuration)
	
	if fatigueDuration > -1:
		fatigueDuration = fatigueDuration - roundsToReduce
		fatigueDuration = max(fatigueDuration, 0)
		
		if fatigueDuration < 1:
			#See if the exhaustion duration should be downgraded to the fatigue duration
			if (exhaustDuration == -1) or (exhaustDuration > 0):
				args.set_arg(1, exhaustDuration)
				args.set_arg(2, 0)
			else:
				args.condition_remove()
		else:
			args.set_arg(1, fatigueDuration)
	return 0
	
def FatigueRemove(attachee, args, evt_obj):
	args.condition_remove()
	return 0
	
def FatigueAddHeal(attachee, args, evt_obj):
	is_heal = evt_obj.is_modifier("sp-Heal")
	is_restore = evt_obj.is_modifier("sp-Restoration")
	is_grestore = evt_obj.is_modifier("sp-Greater Restoration")
	if is_heal or is_restore or is_grestore:
		attachee.float_text_line("Fatigue Removed")
		args.condition_remove()

	if evt_obj.is_modifier("sp-Lesser Restoration"):
		rage_fatigue_dur = args.get_arg(0)
		fatigue_dur = args.get_arg(1)
		exhaust_dur = args.get_arg(2)
		if exhaust_dur != 0:
			args.set_arg(2, 0)
			args.set_arg(1, exhaust_dur)
			attachee.float_text_line("Fatigue Decreased")
		else:
			attachee.float_text_line("Fatigue Removed")
			args.condition_remove()

	return 0
	
def BarbarianFatiguedQuery(attachee, args, evt_obj):
	rageFatigueDuration = args.get_arg(0)
	evt_obj.return_val = (rageFatigueDuration > 0)
	return 0
	
def FatiguedQuery(attachee, args, evt_obj):
	fatigueDuration = args.get_arg(1)
	if (fatigueDuration > 0) or (fatigueDuration == -1):
		evt_obj.return_val = 1
	return 0
	
def ExhaustedQuery(attachee, args, evt_obj):
	exhaustDuration = args.get_arg(2)
	if exhaustDuration != 0:
		evt_obj.return_val = 1
	return 0
	
def AddBarbarianFatigueSignal(attachee, args, evt_obj):
	#Always update the barbarian rage timer
	duration = evt_obj.data1
	args.set_arg(0, duration)
	
	#Update the fatigue duration if this is longer
	fatigueDuration = args.get_arg(1)
	if duration > fatigueDuration and fatigueDuration != -1:
		args.set_arg(1, duration)
	return 0

def AddFatigueSignal(attachee, args, evt_obj):
	if evt_obj.data1 == 0:
		fatigueDurationNew = evt_obj.data1
	else:
		fatigueDurationNew = -1
	
	upgrade = args.get_arg(3)
	exhaustDuration = args.get_arg(2)
	
	#See if an upgrade should be performed
	if exhaustDuration == 0:
		if upgrade:
			args.set_arg(2, fatigueDurationNew)
			game.particles("Fatigue Boom", attachee) #Play only the boom for an upgrade
			attachee.float_text_line("Exhausted")
		else:
			attachee.float_text_line("Already Fatigued")
	else:
		target_item.obj.float_text_line("Already Fatigued")
		
	return 0

def AddExhaustionSignal(attachee, args, evt_obj):
	exhaustOld = args.get_arg(2)
	
	#Data 2 signals a indefinite duration (sending -1 in a signal does not work well)
	if evt_obj.data2 == 0:
		exhaustNew = evt_obj.data1
	else:
		exhaustNew = -1
	
	if exhaustOld == 0:
		args.set_arg(1, exhaustNew)
		args.set_arg(2, exhaustNew)
		game.particles("Fatigue Boom", attachee) #Play only the boom for an upgrade
		attachee.float_text_line("Exhausted")
	else:
		attachee.float_text_line("Already Exhausted")
	
	return 0
	
def FatiguePlayParticlesSaveId(attachee, args, evt_obj):
	partSys = game.particles("Barbarian Fatigue", attachee)
	args.set_arg(4, partSys)
	return 0
	
def FatigueOnRemove(attachee, args, evt_obj):
	partSys = args.get_arg(4)
	game.particles_kill(partSys)
	return 0
	
def FatigueOnRemove2(attachee, args, evt_obj):
	game.particles("Barbarian Fatigue-END", attachee)
	return 0
	
Fatigue = PythonModifier("FatigueExhaust", 6) #Barbarian Fatigue Duration, Fatigue Duration, Exhaustion Duration, Upgradable, Particle System, Spare
Fatigue.AddHook(ET_OnConditionAdd, EK_NONE, FatigueOnAdd, ())
Fatigue.AddHook(ET_OnGetTooltip, EK_NONE, FatigueTooltip, ())
Fatigue.AddHook(ET_OnGetEffectTooltip, EK_NONE, FatigueEffectTooltip, ())
Fatigue.AddHook(ET_OnAbilityScoreLevel, EK_STAT_DEXTERITY, FatigueDexMod, ())
Fatigue.AddHook(ET_OnAbilityScoreLevel, EK_STAT_STRENGTH, FatigueStrMod, ())
Fatigue.AddHook(ET_OnGetMoveSpeed, EK_NONE, ExhaustedMovement, ())
Fatigue.AddHook(ET_OnBeginRound, EK_NONE, FatigueBeginRound, ())
Fatigue.AddHook(ET_OnNewDay, EK_NEWDAY_REST, FatigueRemove, ())
Fatigue.AddHook(ET_OnConditionAddPre, EK_NONE, FatigueAddHeal, ())
Fatigue.AddHook(ET_OnConditionAddFromD20StatusInit, EK_NONE, FatiguePlayParticlesSaveId, ())
Fatigue.AddHook(ET_OnConditionRemove, EK_NONE, FatigueOnRemove, ())
Fatigue.AddHook(ET_OnConditionRemove2, EK_NONE, FatigueOnRemove2, ())
Fatigue.AddHook(ET_OnD20Query, EK_Q_Barbarian_Fatigued, BarbarianFatiguedQuery, ())
Fatigue.AddHook(ET_OnD20Signal, EK_S_Killed, FatigueRemove, ())
Fatigue.AddHook(ET_OnD20PythonQuery, "Fatigued", FatiguedQuery, ())
Fatigue.AddHook(ET_OnD20PythonQuery, "Exhausted", ExhaustedQuery, ())
Fatigue.AddHook(ET_OnD20PythonSignal, "Add Barbarian Fatigue", AddBarbarianFatigueSignal, ())
Fatigue.AddHook(ET_OnD20PythonSignal, "Add Fatigue", AddFatigueSignal, ())
Fatigue.AddHook(ET_OnD20PythonSignal, "Add Exhaustion", AddExhaustionSignal, ())

