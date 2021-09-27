from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Far Shot Feat"

def OnRangeIncrementBonus(attachee, args, evt_obj):
	if (evt_obj.weapon_used and game.is_ranged_weapon(evt_obj.weapon_used.get_weapon_type())):
		coef = 1 if evt_obj.weapon_used.is_throwing_weapon() else 0.5
		evt_obj.range_bonus += int(evt_obj.weapon_used.obj_get_int(obj_f_weapon_range) * coef)
	return 0

farShotFeat = PythonModifier("Far Shot Feat", 3) # 3 - reserved
farShotFeat.MapToFeat(feat_far_shot)
farShotFeat.AddHook(ET_OnRangeIncrementBonus, EK_NONE, OnRangeIncrementBonus, ())
