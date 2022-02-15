from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Bladeweave"

def bladeweaveSpellBeginRound(attachee, args, evt_obj):
    args.set_arg(3, 0) #unset usedFlag
    return 0

def bladeweaveSpellApplyDazeEffect(attachee, args, evt_obj):
    usedFlag = args.get_arg(3)
    if usedFlag:
        return 0
    target = evt_obj.attack_packet.target
    spellId = args.get_arg(0)
    spellDc = args.get_arg(2)
    spellPacket = tpdp.SpellPacket(spellId)
    if not spellPacket.check_spell_resistance(target): #check spell resistance
        #Saving throw to avoid dazed condition
        game.create_history_freeform("{} saves versus ~Dazed~[TAG_DAZED] effect\n\n".format(target.description))
        if target.saving_throw_spell(spellDc, D20_Save_Will, D20STD_F_NONE, spellPacket.caster, spellId): #success
            target.float_text_line("Not Dazed")
            game.particles('Fizzle', target)
        else:
            duration = 1
            particlesId = game.particles('sp-Daze', target)
            spellPacket.add_target(target, particlesId)
            target.condition_add_with_args("sp-Daze", spellId, duration, 0)
    args.set_arg(3, 1) #set usedFlag
    return 0

bladeweaveSpell = SpellPythonModifier("sp-Bladeweave", 5) # spell_id, duration, spellDc, usedFlag, empty
bladeweaveSpell.AddHook(ET_OnBeginRound, EK_NONE, bladeweaveSpellBeginRound,())
bladeweaveSpell.AddHook(ET_OnDealingDamage2, EK_NONE, bladeweaveSpellApplyDazeEffect,())
bladeweaveSpell.AddSpellDismiss()
bladeweaveSpell.AddSpellNoDuplicate()
