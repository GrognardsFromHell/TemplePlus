from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Nightshield"

def nightshieldSpellHasSpellActive(attachee, args, evt_obj):
    if evt_obj.data1 == spell_shield: #replies to query as spell_shield as well to grant Magic Missile immunity
        evt_obj.return_val = 1
    return 0

nightshieldSpell = SpellPythonModifier("sp-Nightshield", 4) # spell_id, duration, spellBonus, empty
nightshieldSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, nightshieldSpellHasSpellActive, ())
nightshieldSpell.AddSaveBonus(passed_by_spell, bonus_type_resistance)
