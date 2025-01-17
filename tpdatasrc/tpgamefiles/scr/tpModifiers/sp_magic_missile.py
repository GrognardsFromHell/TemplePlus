from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import *

print "registering sp_magic_missile"

def GetStatus(caster, args, evt_obj):
	id = evt_obj.data1
	if id != args.get_arg(0): return 0

	ix = evt_obj.data2
	if ix < 1 or 5 < ix: return 0

	evt_obj.return_val = args.get_arg(ix)
	return 0

def Remove(caster, args, evt_obj):
	args.condition_remove()
	return 0

# 0: spell_id
# 1-5: mirror hit projectile 1-5
mmm = PythonModifier('Magic Missile Mirror', 6)
mmm.AddHook(ET_OnD20PythonQuery, 'Missile Mirror Hit', GetStatus, ())
mmm.AddHook(ET_OnD20Signal, EK_S_Spell_End, Remove, ())
