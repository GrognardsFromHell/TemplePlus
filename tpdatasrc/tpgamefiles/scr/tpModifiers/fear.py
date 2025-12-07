from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from cond_utils import CondPythonModifier

print "Registering standalone fear condition"

# Chill touch fear acts as 'panic' where the object flees any source of danger.
# Taking advantage of this to not need to store the caster for now, since
# object handles do not persist across reloads.
def FearTarget(critter, args, evt_obj):
	evt_obj.return_val = 1

	scared_of = OBJ_HANDLE_NULL
	dist = 0
	for other in game.obj_list_vicinity(critter.location, OLC_CRITTERS):
		if critter.is_friendly(other): continue

		odist = critter.distance_to(other)
		if scared_of == OBJ_HANDLE_NULL or odist < dist:
			scared_of = other
			dist = odist

	evt_obj.set_args_from_obj(scared_of)

	return 0

def FloatAfraid(critter, args, evt_obj):
	critter.float_mesfile_line('mes\\spell.mes', 20013, tf_red)
	return 0

def Minus2(critter, args, evt_obj):
	evt_obj.bonus_list.add(-2, 13, 172)
	return 0

def ResetAI(critter, args, evt_obj):
	critter.ai_stop_fleeing()
	return 0

# fear-like effect inflicted on undead due to Chill Touch
#
# arg0 = duration
# arg1 = particles
# arg2-7 = spare, possibly storing caster objid in the future
chill_fear = CondPythonModifier("Chill Touch Fear", 8)
chill_fear.AddBeginParticles()
chill_fear.AddEndParticles()
chill_fear.AddCoalesce()
chill_fear.AddHook(ET_OnConditionAdd, EK_NONE, FloatAfraid, ())
chill_fear.AddHook(ET_OnD20Query, EK_Q_Critter_Is_Afraid, FearTarget, ())
chill_fear.AddHook(ET_OnSaveThrowLevel, EK_NONE, Minus2, ())
chill_fear.AddHook(ET_OnGetSkillLevel, EK_NONE, Minus2, ())
chill_fear.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE, Minus2, ())
chill_fear.AddHook(ET_OnToHitBonus2, EK_NONE, Minus2, ())
chill_fear.AddHook(ET_OnConditionRemove, EK_NONE, ResetAI, ())
