from toee import *
import tpdp
from spell_utils import SpellPythonModifier, applyAttackPacketBonus

print "Registering sp-Rhinos Rush"

def doubleNormalDamage(attachee, args, evt_obj):
    flags = evt_obj.attack_packet.get_flags()
    if flags & D20CAF_CHARGE:
        if not flags & D20CAF_CRITICAL:
            evt_obj.damage_packet.critical_multiplier_apply(2) #this doubles the normal weapon damage
    return 0

rhinosRushSpell = SpellPythonModifier("sp-Rhinos Rush") # spellId, duration, empty
rhinosRushSpell.AddHook(ET_OnDealingDamage, EK_NONE, doubleNormalDamage, ())
#Rhinos Rush deals double damage on non critical hits, but only hightens the multiplier by 1 if it's actually a critical hit!
rhinosRushSpell.AddHook(ET_OnGetCriticalHitExtraDice, EK_NONE, applyAttackPacketBonus, (1, bonus_type_untyped, D20CAF_CHARGE,))
