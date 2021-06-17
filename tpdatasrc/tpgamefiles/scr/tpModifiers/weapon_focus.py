from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import pymod_utils

### Fixes WF Ray, and Unarmed Strike affecting rays

def weaponTypeMatchesAttack(attack_packet, feat_weapon_type):
    wpn = attack_packet.get_weapon_used()
    
    if wpn == OBJ_HANDLE_NULL: # unarmed or ray attack
        item_weapon_type = wt_unarmed_strike_medium_sized_being
        flags = attack_packet.get_flags()
        is_ray_attack = ( (flags & D20CAF_RANGED) != 0 ) and ( (flags & D20CAF_TOUCH_ATTACK) != 0 )

        if feat_weapon_type == wt_ray:
            if not is_ray_attack:
                return 0
        elif feat_weapon_type in [wt_unarmed_strike_medium_sized_being, feat_weapon_focus_unarmed_strike_small_being]: # unarmed attack
            if is_ray_attack:
                return 0
        else:
            return 0
    else:
        item_weapon_type = wpn.obj_get_int(obj_f_weapon_type)
        if item_weapon_type != feat_weapon_type:
            return 0
    return True

def weaponFocusBonusToHit(attachee, args, evt_obj):
    feat = args.get_arg(0)
    feat_weapon_type = args.get_arg(1)
    
    if not weaponTypeMatchesAttack(evt_obj.attack_packet, feat_weapon_type):
        return 0
    
    bonusValue = 1
    bonusType = 0 #ID 0 = Untyped (stacking)
    bonusMesId = 114  #ID 114 in bonus mes: ~Feat~[TAG_FEATS_DES]
    evt_obj.bonus_list.add_from_feat(bonusValue, bonusType, bonusMesId, feat)
    return 0

wfMod = PythonModifier("Weapon_Focus", 2, False) # override Weapon_Focus condition
# map to all sub-feats
for feat in range(feat_weapon_focus_gauntlet, feat_weapon_focus_gauntlet + feat_weapon_focus_count):
    arg_weapon_type = feat - feat_weapon_focus_gauntlet
    wfMod.MapToFeat(feat, feat_cond_arg2 = arg_weapon_type)

wfMod.AddHook(ET_OnConditionAddPre, EK_NONE, pymod_utils.preventDupSameArg, ())
wfMod.AddHook(ET_OnToHitBonus2, EK_NONE, weaponFocusBonusToHit, ())


gwfMod = PythonModifier("Greater_Weapon_Focus", 2, False) # override Greater_Weapon_Focus condition
# map to all sub-feats
for feat in range(feat_greater_weapon_focus_gauntlet, feat_greater_weapon_focus_gauntlet + feat_greater_weapon_focus_count):
    arg_weapon_type = feat - feat_greater_weapon_focus_gauntlet
    gwfMod.MapToFeat(feat, feat_cond_arg2 = arg_weapon_type)

gwfMod.AddHook(ET_OnConditionAddPre, EK_NONE, pymod_utils.preventDupSameArg, ())
gwfMod.AddHook(ET_OnToHitBonus2, EK_NONE, weaponFocusBonusToHit, ())
