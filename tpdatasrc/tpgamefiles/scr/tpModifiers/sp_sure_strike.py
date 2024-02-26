from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Sure Strike"

def applyToHitBonus(attachee, args, evt_obj):
    #Sure Strike adds an Insight Bonus of +1/three casterlevels to the next Attack Roll
    bonusValue = args.get_arg(2)
    bonusType = 18 #ID 18 = Insight
    evt_obj.bonus_list.add(bonusValue ,bonusType ,"~Insight~[TAG_MODIFIER_INSIGHT] : ~Sure Strike~[TAG_SPELLS_SURE_STRIKE]")
    args.remove_spell()
    args.remove_spell_mod()
    return 0

sureStrikeSpell = SpellPythonModifier("sp-Sure Strike", 4) # spellId, duration, bonusValue, empty
sureStrikeSpell.AddHook(ET_OnToHitBonus2, EK_NONE, applyToHitBonus,())
