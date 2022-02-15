from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Ironguts"

def addNauseatedOnSpellEnd(attachee, args, evt_obj):
    duration = 1
    persistentFlag = 0
    attachee.condition_add_with_args("Nauseated", duration, persistentFlag) #When spell expires target is nauseated for 1 round
    return 0

irongutsSpell = SpellPythonModifier("sp-Ironguts") # spellId, duration, empty
irongutsSpell.AddHook(ET_OnConditionRemove, EK_NONE, addNauseatedOnSpellEnd, ())
#Ironguts adds a +5 Alchemical Bonus to Fortitude Saves vs. poison
irongutsSpell.AddSaveBonus(5, bonus_type_alchemical, EK_SAVE_FORTITUDE, D20STD_F_POISON)
