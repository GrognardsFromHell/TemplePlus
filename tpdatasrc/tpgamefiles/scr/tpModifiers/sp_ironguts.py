from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier
print "Registering sp-Ironguts"

def bonusToPoisonSaves(attachee, args, evt_obj):
    if evt_obj.flags & (1 << (D20STD_F_POISON - 1)):
        bonusValue = 5 # Ironguts adds a +5 Alchemical Bonus to Fortitude Saves vs. poison
        bonusType = 151 #ID 151 = Alchemical
        evt_obj.bonus_list.add(bonusValue, bonusType,"~Alchemical~[TAG_MODIFIER_ALCHEMICAL] : ~Ironguts~[TAG_SPELLS_IRONGUTS]")
    return 0

def addNauseatedOnSpellEnd(attachee, args, evt_obj):
    duration = 1
    persistentFlag = 0
    attachee.condition_add_with_args("Nauseated", duration, persistentFlag) #When spell expires target is nauseated for 1 round
    return 0

irongutsSpell = SpellPythonModifier("sp-Ironguts") # spellId, duration, empty
irongutsSpell.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, bonusToPoisonSaves,())
irongutsSpell.AddHook(ET_OnD20Signal, EK_S_Spell_End, addNauseatedOnSpellEnd, ())
