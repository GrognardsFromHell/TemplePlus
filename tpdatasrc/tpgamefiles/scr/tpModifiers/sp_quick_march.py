from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Quick March"

def quickMarchSpellCorrectDurationForCaster(attachee, args, evt_obj):
    #Reduce duration by 1 for caster so it is actually only active in current round
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    if attachee == spellPacket.caster:
        args.set_arg(1, (args.get_arg(1)-1))
    return 0

quickMarchSpell = SpellPythonModifier("sp-Quick March") # spell_id, duration, empty
#Quick March adds 30ft. to movement speed
quickMarchSpell.AddHook(ET_OnConditionAdd, EK_NONE, quickMarchSpellCorrectDurationForCaster,())
quickMarchSpell.AddMovementBonus(30, bonus_type_enhancement)
