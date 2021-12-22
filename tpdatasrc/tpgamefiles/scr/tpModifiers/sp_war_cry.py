from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-War Cry"

def warCrySpellChargeAttackBonus(attachee, args, evt_obj):
    if evt_obj.attack_packet.get_flags() & D20CAF_CHARGE:
        bonusValue = 4 #War Cry is a +4 Morale Bonus to Attack Rolls while charging
        bonusType = 13 #ID 13 = Morale
        evt_obj.bonus_list.add(bonusValue, bonusType, "~Morale~[TAG_MODIFIER_MORALE] : ~War Cry~[TAG_SPELLS_WAR_CRY]")
    return 0

def warCrySpellChargeDamageBonus(attachee, args, evt_obj):
    target = evt_obj.attack_packet.target
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    if evt_obj.attack_packet.get_flags() & D20CAF_CHARGE:
        bonusValue = 4 #War Cry is a +4 Morale Bonus to Damage Rolls while charging
        bonusType = 13 #ID 13 = Morale
        evt_obj.damage_packet.bonus_list.add(4, 13, "~Morale~[TAG_MODIFIER_MORALE] : ~War Cry~[TAG_SPELLS_WAR_CRY]")
        #Saving throw to avoid panicked condition; racial immunity is checked in the condition itself
        #game.create_history_freeform(attachee.description + " saves versus ~War Cry~[TAG_SPELLS_WAR_CRY] panicked effect\n\n")
        #if target.saving_throw_spell(args.get_arg(2), D20_Save_Will, D20STD_F_NONE, spellPacket.caster, args.get_arg(0)): #success
        #    target.float_text_line("Not Panicked")
        #else:
        #    target.condition_add('Panicked Condition', 1, spellPacket.spell_id)
    return 0


warCrySpell = PythonModifier("sp-War Cry", 4) # spell_id, duration, spellDc, empty
warCrySpell.AddHook(ET_OnToHitBonus2, EK_NONE, warCrySpellChargeAttackBonus,())
warCrySpell.AddHook(ET_OnDealingDamage2, EK_NONE, warCrySpellChargeDamageBonus,())
warCrySpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
warCrySpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
warCrySpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
warCrySpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
warCrySpell.AddSpellDispelCheckStandard()
warCrySpell.AddSpellTeleportPrepareStandard()
warCrySpell.AddSpellTeleportReconnectStandard()
warCrySpell.AddSpellCountdownStandardHook()
