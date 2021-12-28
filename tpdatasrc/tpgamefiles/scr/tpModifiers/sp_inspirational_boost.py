from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Inspirational Boost"

def inspirationalBoostSpellBonus(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

def inspirationalBoostSpellOnBeginRound(attachee, args, evt_obj):
    if not args.get_arg(2):
        duration = args.get_arg(1)
        duration += 1
        args.set_arg(1, duration)
    return 0

def inspirationalBoostSpellCheckExpiry(attachee, args, evt_obj):
    roundsToExpire = attachee.d20_query("Bardic Ability Duration Bonus")
    roundsToExpire += 5 #Bard songs linger 5 rounds after song ended
    args.set_arg(1, roundsToExpire)
    args.set_arg(2, 1)
    return 0

inspirationalBoostSpell = SpellPythonModifier("sp-Inspirational Boost", 5) #spell_id, duration, ,expiryFlag, empty, empty
inspirationalBoostSpell.AddHook(ET_OnD20PythonQuery, "Inspirational Boost", inspirationalBoostSpellBonus, ())
inspirationalBoostSpell.AddHook(ET_OnBeginRound, EK_NONE, inspirationalBoostSpellOnBeginRound, ())
inspirationalBoostSpell.AddHook(ET_OnD20Signal, EK_S_Bardic_Music_Completed, inspirationalBoostSpellCheckExpiry, ())
