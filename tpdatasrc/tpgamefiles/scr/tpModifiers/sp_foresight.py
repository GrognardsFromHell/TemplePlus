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
	
def ForesightRemove(attachee, args, evt_obj):
	# Show the remove spell effect
	game.particles( 'sp-Foresight-END', attachee)
	return 0

foresight = PythonModifier("sp-Foresight", 4)
foresight.AddHook(ET_OnGetTooltip, EK_NONE, ForesightTooltip, ())
foresight.AddHook(ET_OnGetEffectTooltip, EK_NONE, ForesightEffectTooltip, ())
foresight.AddHook(ET_OnConditionAddPre, EK_NONE, ConditionImmunityOnPreAdd, ())
foresight.AddHook(ET_OnGetAC, EK_NONE, ForesightAcBonus, ())
foresight.AddHook(ET_OnSaveThrowLevel , EK_SAVE_REFLEX , ForesightReflexSaveBonus, ())
foresight.AddHook(ET_OnConditionRemove, EK_NONE, ForesightRemove, ())
foresight.AddSpellCountdownStandardHook()
