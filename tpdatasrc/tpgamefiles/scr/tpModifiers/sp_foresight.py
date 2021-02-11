from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
print "Registering sp-Foresight"

# args: (0-4)
# 0 - spell_id
# 1 - duration
# 2 - spare
# 3 - spare
	
def ForesightTooltip(attachee, args, evt_obj):
	# Set the tooltip
	evt_obj.append("Foresight (" + str(args.get_arg(1)) + " rounds)")

	return 0
	
def ForesightEffectTooltip(attachee, args, evt_obj):
	# Set the tooltip
	evt_obj.append(tpdp.hash("FORESIGHT"), -2, " (" + str(args.get_arg(1)) + " rounds)")
	return 0

def ConditionImmunityOnPreAdd(attachee, args, evt_obj):
	#Disable Flatfooted
	val = evt_obj.is_modifier("Flatfooted")
	if val:
		print "Removed Flatfooted condition"
		evt_obj.return_val = 0
	
	#Disable Surprised
	val = evt_obj.is_modifier("Surprised")
	if val:
		print "Removed Surprised condition"
		evt_obj.return_val = 0
	return 0
	
def ForesightAcBonus(attachee, args, evt_obj):
	evt_obj.bonus_list.add(2, 0, "Foresight")  #  Insight Bonus
	return 0
	
def ForesightReflexSaveBonus(attachee, args, evt_obj):
	evt_obj.bonus_list.add(2, 0, "Foresight")  #  Insight Bonus
	return 0
	
def ForesightSpellEnd(attachee, args, evt_obj):
	spell_id = args.get_arg(0)
	if evt_obj.data1 == spell_id:
		game.particles( 'sp-Foresight-END', attachee)
	return 0
	
def ForesightHasSpellActive(attachee, args, evt_obj):
	if evt_obj.data1 == 186:
		evt_obj.return_val = 1
	return 0
	
def ForesightKilled(attachee, args, evt_obj):
	args.remove_spell()
	args.remove_spell_mod()
	return 0

foresight = PythonModifier("sp-Foresight", 4)
foresight.AddHook(ET_OnGetTooltip, EK_NONE, ForesightTooltip, ())
foresight.AddHook(ET_OnGetEffectTooltip, EK_NONE, ForesightEffectTooltip, ())
foresight.AddHook(ET_OnConditionAddPre, EK_NONE, ConditionImmunityOnPreAdd, ())
foresight.AddHook(ET_OnGetAC, EK_NONE, ForesightAcBonus, ())
foresight.AddHook(ET_OnSaveThrowLevel , EK_SAVE_REFLEX , ForesightReflexSaveBonus, ())
foresight.AddHook(ET_OnD20Signal, EK_S_Spell_End, ForesightSpellEnd, ())
foresight.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, ForesightHasSpellActive, ())
foresight.AddHook(ET_OnD20Signal, EK_S_Killed, ForesightKilled, ())
foresight.AddSpellDispelCheckStandard()
foresight.AddSpellTeleportPrepareStandard()
foresight.AddSpellTeleportReconnectStandard()
foresight.AddSpellCountdownStandardHook()
