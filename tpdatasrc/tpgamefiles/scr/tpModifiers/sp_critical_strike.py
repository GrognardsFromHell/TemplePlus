from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Critical Strike"

def criticalStrikeSpellModifyThreatRange(attachee, args, evt_obj):
    if evt_obj.attack_packet.get_weapon_used().obj_get_int(obj_f_type) == obj_t_weapon: #Keen requires weapon
        weaponKeenRange = evt_obj.attack_packet.get_weapon_used().obj_get_int(obj_f_weapon_crit_range)
        bonusType = 12 #ID 12 = Enhancement
        evt_obj.bonus_list.add(weaponKeenRange, bonusType, "~Insight~[TAG_MODIFIER_INSIGHT] : ~Critical Strike~[TAG_SPELLS_CRITICAL_STRIKE]")
    return 0

def criticalStrikeSpellBonusToConfirmCrit(attachee, args, evt_obj):
    bonus = 4 #+4 to confirm crits
    bonusType = 18 #ID 18 = Insight
    evt_obj.bonus_list.add(bonus, bonusType,"~Insight~[TAG_MODIFIER_INSIGHT] : ~Critical Strike~[TAG_SPELLS_CRITICAL_STRIKE]")
    return 0

def criticalStrikeSpellBonusToDamage(attachee, args, evt_obj):
    target =  evt_obj.attack_packet.target
    #Check if opponent is immnue to precision damage
    if (spell_utils.checkCategoryType(target, mc_type_construct, mc_type_ooze, mc_type_plant, mc_type_undead)
    or target.is_category_subtype(mc_subtype_incorporeal)):
        return 0
    else:
        #target needs to be denied its dexterity bonus to AC for whatever reason or be flanked
        if target.d20_query(Q_Helpless) == 1 or target.d20_query(Q_Flatfooted) == 1 or (evt_obj.attack_packet.get_flags() & D20CAF_FLANKED):
            bonusDice = dice_new('1d6') #Critical Strike Bonus Damage
            damageType = D20DT_UNSPECIFIED
            damageMesId = 3000 #ID3000 added in damage.mes
            evt_obj.damage_packet.add_dice(bonusDice, damageType, damageMesId)
    return 0

criticalStrikeSpell = PythonModifier("sp-Critical Strike", 2) # spell_id, duration
criticalStrikeSpell.AddHook(ET_OnGetCriticalHitRange, EK_NONE, criticalStrikeSpellModifyThreatRange,())
criticalStrikeSpell.AddHook(ET_OnConfirmCriticalBonus, EK_NONE, criticalStrikeSpellBonusToConfirmCrit,())
criticalStrikeSpell.AddHook(ET_OnDealingDamage, EK_NONE, criticalStrikeSpellBonusToDamage,())
criticalStrikeSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
criticalStrikeSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
criticalStrikeSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
criticalStrikeSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
criticalStrikeSpell.AddSpellDispelCheckStandard()
criticalStrikeSpell.AddSpellTeleportPrepareStandard()
criticalStrikeSpell.AddSpellTeleportReconnectStandard()
criticalStrikeSpell.AddSpellCountdownStandardHook()
