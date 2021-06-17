from templeplus.pymod import PythonModifier
from toee import *
import tpdp

### WF Ray Fix ###

def weaponFocusRayBonusToHit(attachee, args, evt_obj):
    if not evt_obj.attack_packet.get_flags() & D20CAF_TOUCH_ATTACK:
        return 0
    elif not evt_obj.attack_packet.get_flags() & D20CAF_RANGED:
        return 0
    elif attachee.has_feat(feat_weapon_focus_unarmed_strike_medium_sized_being): #workaround to not stack with wf_unarmed
        return 0
    elif attachee.has_feat(feat_weapon_focus_unarmed_strike_small_being): #workaround to not stack with wf_unarmed
        return 0
    else:
        bonusValue = 1
        bonusType = 0 #ID 0 = Untyped (stacking)
        bonusMesId = 114  #ID 114 in bonus mes: ~Feat~[TAG_FEATS_DES]
        evt_obj.bonus_list.add_from_feat(bonusValue, bonusType, bonusMesId, feat_weapon_focus_ray)
    return 0

wfRayFix = PythonModifier("Weapon Focus Ray Fix", 0)
wfRayFix.MapToFeat(feat_weapon_focus_ray)
wfRayFix.AddHook(ET_OnToHitBonus2, EK_NONE, weaponFocusRayBonusToHit, ())
