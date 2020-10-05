from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Prayer Beads"

def PrBeadsNewday(attachee, args, evt_obj):
	args.set_arg(0, 0)
	args.set_arg(1, 0)
	args.set_arg(3, 0)
	return 0

def PrBeadsRadial(attachee, args, evt_obj):
	invIdx = args.get_arg(2)
	radialAction = tpdp.RadialMenuEntryAction("Prayer Beads (Karma)", D20A_ACTIVATE_DEVICE_FREE, invIdx, "TAG_INTERFACE_HELP")
	radialAction.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Items)
	return 0

def PrBeadsPerform(attachee, args, evt_obj):

	invIdx = args.get_arg(2)
	if invIdx == evt_obj.d20a.data1:
		numUsedToday = args.get_arg(3)
		args.set_arg(3, numUsedToday + 1)
		attachee.condition_add_with_args("Prayer Beads Karma Effect", 0, 0)
	return 0

def PrBeadsCheck(attachee, args, evt_obj):
	invIdx = args.get_arg(2)
	if invIdx == evt_obj.d20a.data1:
		numUsedToday = args.get_arg(3)
		if numUsedToday > 0:
			evt_obj.return_val = AEC_OUT_OF_CHARGES
	return 0


prbd = PythonModifier("Prayer Beads", 5) # arg2 is the inventory index (automatically set by the game), arg3 is times used this day, arg4 is reserved
prbd.AddHook(ET_OnNewDay, EK_NEWDAY_REST, PrBeadsNewday, ())
prbd.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, PrBeadsRadial, ())
prbd.AddHook(ET_OnD20ActionCheck, EK_D20A_ACTIVATE_DEVICE_FREE, PrBeadsCheck, ())
prbd.AddHook(ET_OnD20ActionPerform, EK_D20A_ACTIVATE_DEVICE_FREE, PrBeadsPerform, ())
prbd.AddItemForceRemoveHandler()




prayerKarma = PythonModifier("Prayer Beads Karma Effect", 3)

def PrBeadsAdded(attachee, args, evt_obj):
	args.set_arg(2, 100)
	return 0

def PrBeadsCasterLevelBonus(attachee, args, evt_obj):
	spellPkt = evt_obj.get_spell_packet()
	if spellPkt.is_divine_spell():
		evt_obj.return_val += 4
	return 0

def PrBeadsTickdown(attachee, args, evt_obj):
	numRounds = args.get_arg(2)
	roundsToReduce = evt_obj.data1
	if numRounds - roundsToReduce >= 0:
		args.set_arg(2, numRounds - roundsToReduce)
		return 0
	else:
		args.condition_remove()
		return 0

	return 0

def PrayerEffectTooltip(attachee, args, evt_obj):
	evt_obj.append(tpdp.hash("PRAYER_BEADS_KARMA"), -2, "Karmic Prayer (" + str(args.get_arg(2)) + " rounds)")
	return 0



prayerKarma.AddHook(ET_OnGetCasterLevelMod, EK_NONE, PrBeadsCasterLevelBonus, ())
prayerKarma.AddHook(ET_OnBeginRound, EK_NONE, PrBeadsTickdown, ())
prayerKarma.AddHook(ET_OnConditionAdd, EK_NONE, PrBeadsAdded, ())
prayerKarma.AddHook(ET_OnGetEffectTooltip, EK_NONE, PrayerEffectTooltip, ())