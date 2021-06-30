from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Nightshield"

def nightshieldSpellBonusToSaves(attachee, args, evt_obj):
    bonusValue = args.get_arg(2) #Bonus is passed by spell
    bonusType = 15 # ID 15 = Resistance
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Resistance~[TAG_MODIFIER_RESISTANCE] Bonus : ~Nightshield~[TAG_SPELLS_NIGHTSHIELD]")
    return 0

def nightshieldSpellHasSpellActive(attachee, args, evt_obj):
    if evt_obj.data1 == spell_shield: #replies to query as spell_shield as well to grant Magic Missile immunity
        evt_obj.return_val = 1
    return 0

nightshieldSpell = PythonModifier("sp-Nightshield", 4, False) # spell_id, duration, spellBonus, empty
nightshieldSpell.AddHook(ET_OnSaveThrowLevel, EK_NONE, nightshieldSpellBonusToSaves, ())
nightshieldSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, nightshieldSpellHasSpellActive, ())
nightshieldSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
nightshieldSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
nightshieldSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
nightshieldSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
nightshieldSpell.AddSpellDispelCheckStandard()
nightshieldSpell.AddSpellTeleportPrepareStandard()
nightshieldSpell.AddSpellTeleportReconnectStandard()
nightshieldSpell.AddSpellCountdownStandardHook()
