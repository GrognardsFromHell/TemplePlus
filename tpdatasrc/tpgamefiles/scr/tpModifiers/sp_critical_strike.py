from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier, getSpellHelpTag

print "Registering sp-Critical Strike"

def verifyCriticalStrikeConditions(attackPacket):
    flags = attackPacket.get_flags()
    weaponUsed = attackPacket.get_weapon_used()
    target = attackPacket.target
    attacker = attackPacket.attacker

    if weaponUsed.obj_get_int(obj_f_type) != obj_t_weapon:
        return False
    elif attacker.can_sneak_attack(target):
        return True
    return False


def criticalStrikeSpellModifyThreatRange(attachee, args, evt_obj):
    attackPacket = evt_obj.attack_packet
    if verifyCriticalStrikeConditions(attackPacket):
        weaponKeenRange = attackPacket.get_weapon_used().obj_get_int(obj_f_weapon_crit_range)
        bonusType = bonus_type_enhancement
        bonusHelpTag = game.get_mesline("mes\\bonus_description.mes", bonusType)
        spellId = args.get_arg(0)
        spellHelpTag = getSpellHelpTag(spellId)
        evt_obj.bonus_list.add(weaponKeenRange, bonusType, "{} : {}".format(bonusHelpTag, spellHelpTag))
    return 0

def criticalStrikeSpellBonusToConfirmCrit(attachee, args, evt_obj):
    attackPacket = evt_obj.attack_packet
    if verifyCriticalStrikeConditions(attackPacket):
        bonusValue = 4 #+4 to confirm crits
        bonusType = bonus_type_insight
        bonusHelpTag = game.get_mesline("mes\\bonus_description.mes", bonusType)
        spellId = args.get_arg(0)
        spellHelpTag = getSpellHelpTag(spellId)
        evt_obj.bonus_list.add(bonusValue, bonusType, "{} : {}".format(bonusHelpTag, spellHelpTag))
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
