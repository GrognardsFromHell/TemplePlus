from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier, getSpellHelpTag

print "Registering sp-War Cry"

def warCrySpellChargeAttackBonus(attachee, args, evt_obj):
    if evt_obj.attack_packet.get_flags() & D20CAF_CHARGE:
        bonusValue = 4 #War Cry is a +4 Morale Bonus to Attack Rolls while charging
        bonusType = bonus_type_morale
        bonusHelpTag = game.get_mesline("mes\\bonus_description.mes", bonusType)
        spellId = args.get_arg(0)
        spellHelpTag = getSpellHelpTag(spellId)
        evt_obj.bonus_list.add(bonusValue, bonusType, "{} : {}".format(bonusHelpTag, spellHelpTag))
    return 0

def warCrySpellChargeDamageBonus(attachee, args, evt_obj):
    if evt_obj.attack_packet.get_flags() & D20CAF_CHARGE:
        bonusValue = 4 #War Cry is a +4 Morale Bonus to Damage Rolls while charging
        bonusType = bonus_type_morale
        bonusHelpTag = game.get_mesline("mes\\bonus_description.mes", bonusType)
        spellId = args.get_arg(0)
        spellHelpTag = getSpellHelpTag(spellId)
        evt_obj.damage_packet.bonus_list.add(bonusValue, bonusType, "{} : {}".format(bonusHelpTag, spellHelpTag))
        #Saving throw to avoid panicked condition
        target = evt_obj.attack_packet.target
        spellPacket = tpdp.SpellPacket(spellId)
        spellDc = args.get_arg(2)
        if spellPacket.check_spell_resistance(target):
            return 0
        game.create_history_freeform("{} saves versus ~War Cry~[TAG_SPELLS_WAR_CRY] panicked effect\n\n".format(target.description))
        if target.saving_throw_spell(spellDc, D20_Save_Will, D20STD_F_NONE, spellPacket.caster, spellId):
            target.float_mesfile_line("mes\\spell.mes", 30001)
        else:
            target.float_mesfile_line("mes\\spell.mes", 30002)
            particlesId = game.particles("sp-Fear-Hit", target)
            if spellPacket.add_target(target, particlesId):
                #target.float_text_line("Panicked!", tf_red)
                duration = 1
                isFeared = 0
                target.condition_add("sp-Fear", spellId, duration, isFeared)
                dropFlag = 1
                if target.item_worn_at(item_wear_weapon_primary) != OBJ_HANDLE_NULL:
                    target.item_worn_unwield(item_wear_weapon_primary, dropFlag)
                if target.item_worn_at(item_wear_weapon_secondary) != OBJ_HANDLE_NULL:
                    target.item_worn_unwield(item_wear_weapon_secondary, dropFlag)
                if target.item_worn_at(item_wear_shield) != OBJ_HANDLE_NULL:
                    target.item_worn_unwield(item_wear_shield, dropFlag)
    return 0

warCrySpell = SpellPythonModifier("sp-War Cry", 4) #spellId, duration, spellDc, empty
warCrySpell.AddHook(ET_OnToHitBonus2, EK_NONE, warCrySpellChargeAttackBonus,())
warCrySpell.AddHook(ET_OnDealingDamage2, EK_NONE, warCrySpellChargeDamageBonus,())
