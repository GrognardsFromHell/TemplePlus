from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Grace"

def addGoodProperty(attachee, args, evt_obj):
    if evt_obj.attack_packet.get_flags() & D20CAF_RANGED:
        return 0
    elif evt_obj.damage_packet.attack_power & D20DAP_HOLY:
        return 0
    elif evt_obj.damage_packet.attack_power & D20DAP_UNHOLY:
        evt_obj.damage_packet.attack_power -= D20DAP_UNHOLY
    elif evt_obj.damage_packet.attack_power & D20DAP_LAW:
        evt_obj.damage_packet.attack_power -= D20DAP_LAW
    elif evt_obj.damage_packet.attack_power & D20DAP_CHAOS:
        evt_obj.damage_packet.attack_power -= D20DAP_CHAOS
    evt_obj.damage_packet.attack_power |= D20DAP_HOLY
    return 0

graceSpell = SpellPythonModifier("sp-Grace") # spellId, duration, empty
graceSpell.AddSkillBonus(-20, bonus_type_circumstance, skill_hide)
graceSpell.AddAbilityBonus(2, bonus_type_sacred, stat_dexterity)
graceSpell.AddMovementBonus(10, bonus_type_untyped)
graceSpell.AddHook(ET_OnDealingDamage, EK_NONE, addGoodProperty, ())
graceSpell.AddSpellNoDuplicate()
