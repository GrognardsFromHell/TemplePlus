from templeplus.pymod import PythonModifier
from toee import *
import tpdp

def Remove(char, args, evt_obj):
	if evt_obj.is_modifier('Countersong'):
		args.condition_remove()
	return 0

countersong = PythonModifier()
countersong.ExtendExisting('Countersong')
countersong.AddHook(ET_OnConditionAddPre, EK_NONE, Remove, ())
