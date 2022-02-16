from toee import *
import tpdp
from spell_utils import TouchModifier

print "Registering sp-Bolts of Bedevilment"

def dealTouchDamage(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)

    touchAction = evt_obj.get_d20_action()
    target = touchAction.target

    attachee.float_mesfile_line('mes\\combat.mes', 68) #ID 68 = Touch Attack Hit!

    durationDice = dice_new("1d3")
    dazeDuration = durationDice.roll()
    dazeDc = args.get_arg(3)
    if target.saving_throw(dazeDc, D20_Save_Will, D20STD_F_SPELL_DESCRIPTOR_MIND_AFFECTING, attachee, D20A_CAST_SPELL): #successful save
        target.float_mesfile_line('mes\\spell.mes', 30001)
    else:
        target.float_mesfile_line('mes\\spell.mes', 30002)
        particlesId = game.particles("sp-Daze", target)
        spellPacket.add_target(target, particlesId)
        target.condition_add_with_args("sp-Daze", spellId, dazeDuration, 0)
    return 0

boltsOfBedevilmentSpell = TouchModifier("sp-Bolts of Bedevilment", 2) # spell_id, duration, numberOfCharges, spell.dc, empty
boltsOfBedevilmentSpell.AddTouchHook(dealTouchDamage)
boltsOfBedevilmentSpell.AddSpellCountdownStandardHook()
