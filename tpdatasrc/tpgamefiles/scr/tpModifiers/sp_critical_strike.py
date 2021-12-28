from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Critical Strike"

def verifyCriticalStrikeConditions(attackPacket):
    flags = attackPacket.get_flags()
    weaponUsed = attackPacket.get_weapon_used()
    target = attackPacket.target
    attacker = attackPacket.attacker
    #if target is blind, it should be sneakable, but both
    #target.d20_query(Q_SneakAttack) and
    #target.d20_query_with_object(Q_OpponentSneakAttack, attacker)
    #return 0; added extra check for blind due to this
    if weaponUsed.obj_get_int(obj_f_type) != obj_t_weapon:
        return False
    elif target.d20_query(Q_SneakAttack):
        return True
    elif target.d20_query_with_object(Q_OpponentSneakAttack, attacker):
        return True
    elif flags & D20CAF_FLANKED:
        return True
    elif target.d20_query(Q_Critter_Is_Blinded):
        return True
    elif attacker.d20_query(Q_Critter_Is_Invisible) and not target.d20_query(Q_Critter_Can_See_Invisible):
        return True
    return False


def criticalStrikeSpellModifyThreatRange(attachee, args, evt_obj):
    attackPacket = evt_obj.attack_packet
    if verifyCriticalStrikeConditions(attackPacket):
        weaponKeenRange = attackPacket.get_weapon_used().obj_get_int(obj_f_weapon_crit_range)
        bonusType = 12 #ID 12 = Enhancement
        evt_obj.bonus_list.add(weaponKeenRange, bonusType, "~Insight~[TAG_MODIFIER_INSIGHT] : ~Critical Strike~[TAG_SPELLS_CRITICAL_STRIKE]")
    return 0

def criticalStrikeSpellBonusToConfirmCrit(attachee, args, evt_obj):
    attackPacket = evt_obj.attack_packet
    if verifyCriticalStrikeConditions(attackPacket):
        bonus = 4 #+4 to confirm crits
        bonusType = 18 #ID 18 = Insight
        evt_obj.bonus_list.add(bonus, bonusType,"~Insight~[TAG_MODIFIER_INSIGHT] : ~Critical Strike~[TAG_SPELLS_CRITICAL_STRIKE]")
    return 0

def criticalStrikeSpellBonusToDamage(attachee, args, evt_obj):
    attackPacket = evt_obj.attack_packet
    if verifyCriticalStrikeConditions(attackPacket):
        target =  attackPacket.target
        #Check if opponent is immnue to precision based damage
        if target.d20_query(Q_Critter_Is_Immune_Critical_Hits):
            #evt_obj.damage_packet.bonus_list.add_zeroed(325) #ID 325 = Creature Immune to ~Sneak Attack~[TAG_CLASS_FEATURES_ROGUE_SNEAK_ATTACK]
            evt_obj.damage_packet.bonus_list.add_zeroed(375) #ID 375 = NEW!
        else:
            bonusDice = dice_new('1d6') #Critical Strike Bonus Damage
            damageType = D20DT_UNSPECIFIED
            damageMesId = 3000 #ID3000 added in damage.mes
            evt_obj.damage_packet.add_dice(bonusDice, damageType, damageMesId)
    else:
        evt_obj.damage_packet.bonus_list.add_zeroed(376) #ID 376 = NEW!
    return 0

criticalStrikeSpell = SpellPythonModifier("sp-Critical Strike") # spell_id, duration, empty
criticalStrikeSpell.AddHook(ET_OnGetCriticalHitRange, EK_NONE, criticalStrikeSpellModifyThreatRange,())
criticalStrikeSpell.AddHook(ET_OnConfirmCriticalBonus, EK_NONE, criticalStrikeSpellBonusToConfirmCrit,())
criticalStrikeSpell.AddHook(ET_OnDealingDamage, EK_NONE, criticalStrikeSpellBonusToDamage,())
