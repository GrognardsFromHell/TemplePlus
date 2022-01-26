from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier, getSpellHelpTag
print "Registering sp-Ironguts"

def bonusToPoisonSaves(attachee, args, evt_obj):
    if evt_obj.flags & (1 << (D20STD_F_POISON - 1)):
        bonusValue = 5 # Ironguts adds a +5 Alchemical Bonus to Fortitude Saves vs. poison
        bonusType = bonus_type_alchemical
        bonusHelpTag = game.get_mesline("mes\\bonus_description.mes", bonusType)
        spellId = args.get_arg(0)
        spellHelpTag = getSpellHelpTag(spellId)
        evt_obj.bonus_list.add(bonusValue, bonusType, "{} : {}".format(bonusHelpTag, spellHelpTag))
    return 0

def addNauseatedOnSpellEnd(attachee, args, evt_obj):
    duration = 1
    persistentFlag = 0
    attachee.condition_add_with_args("Nauseated", duration, persistentFlag) #When spell expires target is nauseated for 1 round
    return 0

irongutsSpell = SpellPythonModifier("sp-Ironguts") # spellId, duration, empty
irongutsSpell.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, bonusToPoisonSaves,())
irongutsSpell.AddHook(ET_OnD20Signal, EK_S_Spell_End, addNauseatedOnSpellEnd, ())
