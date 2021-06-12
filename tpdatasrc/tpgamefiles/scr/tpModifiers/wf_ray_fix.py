from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
print "Registering WF Ray Fix"

#######   Weapon Focus Ray Fix   #######
def wfRayFixAddToHitBonus(attachee, args, evt_obj):
    if not evt_obj.attack_packet.get_flags() & D20CAF_TOUCH_ATTACK:
        return 0
    if not evt_obj.attack_packet.get_flags() & D20CAF_RANGED:
        return 0
    #add_from_feat does not allow freetext
    if attachee.has_feat(feat_greater_weapon_focus_ray):
        evt_obj.bonus_list.add(2, 0, "Feat: ~Greater Weapon Foucs (Ray)~[TAG_WEAPON_FOCUS]")
    elif attachee.has_feat(feat_weapon_focus_ray):
        evt_obj.bonus_list.add(1, 0, "Feat: ~Weapon Foucs (Ray)~[TAG_WEAPON_FOCUS]")
    return 0
####### Weapon Focus Ray Fix END #######

def wfRayFixTickdown(attachee, args, evt_obj):
    args.set_arg(0, args.get_arg(0)-evt_obj.data1) # Ticking down duration
    if args.get_arg(0) < 0:
        args.condition_remove()
    return 0

wfRayFix = PythonModifier("Wf Ray Fix", 1) #duration
wfRayFix.AddHook(ET_OnToHitBonus2, EK_NONE, wfRayFixAddToHitBonus, ())
wfRayFix.AddHook(ET_OnBeginRound, EK_NONE, wfRayFixTickdown, ())
