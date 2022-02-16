from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier, applyBonus

print "Registering sp-Phantom Foe"

def phantomFoeSpellBeginRound(attachee, args, evt_obj):
    if not any(partyMember.can_melee(attachee) for partyMember in game.party):
        attachee.float_text_line("No longer threatened")
        args.remove_spell()
        args.remove_spell_mod()
    return 0

def phantomFoeSpellSetFlankedCondition(attachee, args, evt_obj):
    attacker = evt_obj.attack_packet.attacker
    flags = evt_obj.attack_packet.get_flags()
    weaponUsed = evt_obj.attack_packet.get_weapon_used()
    if flags & D20CAF_RANGED:
        return 0
    elif flags & D20CAF_FLANKED:
        return 0
    elif not attachee.d20_query_with_object(Q_CanBeFlanked, attacker):
        return 0
    flags |= D20CAF_FLANKED
    evt_obj.attack_packet.set_flags(flags)
    attachee.float_text_line("Phantom Foe", tf_red)
    bonusValue = 2
    bonusType = bonus_type_untyped
    bonusMesId = 201 #ID201 is flanked in bonus.mes
    evt_obj.bonus_list.add(bonusValue, bonusType, bonusMesId)
    return 0

phantomFoeSpell = SpellPythonModifier("sp-Phantom Foe") # spell_id, duration, empty
phantomFoeSpell.AddHook(ET_OnBeginRound, EK_NONE, phantomFoeSpellBeginRound, ())
phantomFoeSpell.AddHook(ET_OnToHitBonusFromDefenderCondition, EK_NONE, phantomFoeSpellSetFlankedCondition,())
phantomFoeSpell.AddHook(ET_OnGetAttackerConcealmentMissChance, EK_NONE, applyBonus, (50, bonus_type_concealment,))
