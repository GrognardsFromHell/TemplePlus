from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Blessed Aim"

def blessedAimSpellBonus(attachee, args, evt_obj):
    if evt_obj.attack_packet.get_flags() & D20CAF_RANGED:
        bonusValue = 2
        bonusType = 13 #ID 13 = Morale
        evt_obj.bonus_list.add(bonusValue, bonusType, "~Blessed Aim~[~Morale~[TAG_MODIFIER_MORALE] : TAG_SPELLS_BLESSED_AIM]")
    return 0

blessedAimSpell = PythonModifier("sp-Blessed Aim", 3, False) # spell_id, duration, empty
blessedAimSpell.AddHook(ET_OnToHitBonus2, EK_NONE, blessedAimSpellBonus,())
blessedAimSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
blessedAimSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
blessedAimSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
blessedAimSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
blessedAimSpell.AddSpellDispelCheckStandard()
blessedAimSpell.AddSpellTeleportPrepareStandard()
blessedAimSpell.AddSpellTeleportReconnectStandard()
blessedAimSpell.AddSpellCountdownStandardHook()
