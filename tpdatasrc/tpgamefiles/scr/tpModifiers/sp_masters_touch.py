from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Masters Touch"

def mastersTouchSpellNullifyProficiencyPenalty(attachee, args, evt_obj):
    weaponType = args.get_arg(2)
    if weaponType > -1: #-1 indicates shield
        usedWeapon = evt_obj.attack_packet.get_weapon_used()
        usedWeaponType = usedWeapon.get_weapon_type()
        if usedWeaponType == weaponType:
            ### Workaround modify ###
            evt_obj.bonus_list.modify(4, 37, 138) #negates non proficiency penalty
            ### Workaround modify ###
    else:
            ### Workaround modify ###
            wornShieldArmorCheckPenalty = attachee.item_worn_at(item_wear_shield).obj_get_int(obj_f_armor_armor_check_penalty) # Get Armor Check Penalty
            evt_obj.bonus_list.modify(abs(wornShieldArmorCheckPenalty), 0, 138)
            ### Workaround modify ###
    return 0

mastersTouchSpell = SpellPythonModifier("sp-Masters Touch", 4) # spell_id, duration, weaponType, empty
mastersTouchSpell.AddHook(ET_OnToHitBonus2, EK_NONE, mastersTouchSpellNullifyProficiencyPenalty,())
