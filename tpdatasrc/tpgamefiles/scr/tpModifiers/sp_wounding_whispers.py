from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Wounding Whispers"

def woundingWhispersSpellDealDamage(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    attacker = evt_obj.attack_packet.attacker
    spellDamageDice = dice_new("1d6")
    spellDamageDice.bonus = spellPacket.caster_level
    damageType = D20DT_SONIC
    if attacker.distance_to(attachee) < 6: #Only melee & natural attacks are affected, reach weapons are save
        if not spellPacket.check_spell_resistance(attacker):
            attachee.float_text_line("Wounding Whispers")
            attacker.spell_damage(attachee, damageType, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spellId)
    return 0

woundingWhispersSpell = SpellPythonModifier("sp-Wounding Whispers") #spellId, duration, empty
woundingWhispersSpell.AddHook(ET_OnTakingDamage2, EK_NONE, woundingWhispersSpellDealDamage,())
