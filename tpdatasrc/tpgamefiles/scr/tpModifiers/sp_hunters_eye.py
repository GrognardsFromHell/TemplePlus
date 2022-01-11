from templeplus.pymod import PythonModifier
from toee import *
from spell_utils import SpellPythonModifier

print "Registering sp-Hunter's Eye"

def querySneakDice(attachee, args, evt_obj):
    bonusDice = args.get_arg(2)
    evt_obj.return_val += bonusDice
    return 0

huntersEyeSpell = SpellPythonModifier("sp-Hunter's Eye", 4) # spellId, duration, bonusDice, empty
huntersEyeSpell.AddHook(ET_OnD20PythonQuery, "Sneak Attack Dice", querySneakDice, ())
