from templeplus.pymod import PythonModifier
from toee import *
import tpdp

def Remove(char, args, evt_obj):
	if evt_obj.is_modifier('Countersong'):
		args.condition_remove()
	return 0

# built-in hook only checks for Sonic descriptor
def Lang(char, args, evt_obj):
	lang = 1 << (D20STD_F_SPELL_DESCRIPTOR_LANGUAGE_DEPENDENT-1)
	sonic = 1 << (D20S%D_F_SPELL_DESCRIPTOR_SONIC-1)

	if (evt_obj.flags & lang) and not (evt_obj.flags & sonic):
		perform = args.get_arg(1)
		save_bonus = evt_obj.bonus_list.get_sum()
		delta = perform - save_bonus - evt_obj.roll_result
		if delta > 0:
			evt_obj.bonus_list.add(delta, 0, 192)

	return 0

countersong = PythonModifier()
countersong.ExtendExisting('Countersong')
countersong.AddHook(ET_OnConditionAddPre, EK_NONE, Remove, ())
countersong.AddHook(ET_OnCountersongSaveThrow, EK_NONE, Lang, ())
