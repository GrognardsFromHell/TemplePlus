from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Distort Speech"

def distortCheck(attachee, args, evt_obj):
    spellPacket = evt_obj.get_spell_packet()
    requiredComponents = spellPacket.get_spell_component_flags()
    metaMagicUsed = spellPacket.get_metamagic_data()
    if requiredComponents & SCF_VERBAL: #Only spells with verbal components get distortet; also silent spells should be included; ToDo!
        distortBonusList = tpdp.BonusList()
        failDc = 50 #Distort Speech is a 50% Chance to fail spells and activate items(items are missing atm!)
        failDice = dice_new('1d100')
        distortDiceResult = failDice.roll()
        distortHistory = tpdp.create_history_dc_roll(attachee, failDc, failDice, distortDiceResult, "Distort Spell Failure Check", distortBonusList)
        game.create_history_from_id(distortHistory)
        if distortDiceResult <= failDc:
            evt_obj.return_val = 100
            attachee.float_text_line("Distort Speech Failure", tf_red)
            game.particles('Fizzle', attachee)
    return 0

distortSpeechSpell = SpellPythonModifier("sp-Distort Speech") # spell_id, duration, empty
distortSpeechSpell.AddHook(ET_OnD20Query, EK_Q_SpellInterrupted, distortCheck,())
