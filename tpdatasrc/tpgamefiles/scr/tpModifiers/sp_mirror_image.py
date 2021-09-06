from templeplus.pymod import PythonModifier
from toee import *
print "Registering sp_mirror_image"

def Override(char, args, evt_obj):
	if evt_obj.is_modifier('sp-Greater Mirror Image'):
		args.remove_spell()
		args.remove_spell_mod()

	return 0

cond = PythonModifier()
cond.ExtendExisting('sp-Mirror Image')
cond.AddHook(ET_OnConditionAddPre, EK_NONE, Override, ())
