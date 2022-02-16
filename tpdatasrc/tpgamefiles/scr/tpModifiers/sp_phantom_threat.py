from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Phantom Threat"

def phantomThreatSpellSetFlankedCondition(attachee, args, evt_obj):
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
    attachee.float_text_line("Phantom Threat", tf_red)
    bonusValue = 2
    bonusType = bonus_type_untyped
    bonusMesId = 201 #ID201 is flanked in bonus.mes
    evt_obj.bonus_list.add(bonusValue, bonusType, bonusMesId)
    return 0

phantomThreatSpell = SpellPythonModifier("sp-Phantom Threat") # spell_id, duration, empty
phantomThreatSpell.AddHook(ET_OnToHitBonusFromDefenderCondition, EK_NONE, phantomThreatSpellSetFlankedCondition,())
