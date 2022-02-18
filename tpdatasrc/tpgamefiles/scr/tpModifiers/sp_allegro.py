from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier, getBonusHelpTag, getSpellHelpTag

print "Registering sp-Allegro"

def allegroSpellMovementBonus(attachee, args, evt_obj):
    #Allegro adds 30ft. to movement speed, but is capped at double original speed.
    bonusValue = min(attachee.stat_level_get(stat_movement_speed), 30)
    bonusType = bonus_type_enhancement
    bonusHelpTag = getBonusHelpTag(bonusType)
    spellId = args.get_arg(0)
    spellHelpTag = getSpellHelpTag(spellId)
    evt_obj.bonus_list.add(bonusValue, bonusType, "{} : {}".format(bonusHelpTag, spellHelpTag))
    return 0

allegroSpell = SpellPythonModifier("sp-Allegro") # spell_id, duration, empty
allegroSpell.AddHook(ET_OnGetMoveSpeedBase, EK_NONE, allegroSpellMovementBonus,())
