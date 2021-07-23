from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Quick March"

def quickMarchSpellCorrectDurationForCaster(attachee, args, evt_obj):
    #Reduce duration by 1 for caster so it is actually only active in current round
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    if attachee == spellPacket.caster:
        args.set_arg(1, (args.get_arg(1)-1))
    return 0

def quickMarchSpellMovementBonus(attachee, args, evt_obj):
    bonusValue = 30 #Quick March adds 30ft. to movement speed
    bonusType = 12 #ID 12 = Enhancement
    evt_obj.bonus_list.add(bonusValue, bonusType ,"~Quick March~[TAG_SPELLS_QUICK_MARCH] ~Enhancement~[TAG_ENHANCEMENT] Bonus")
    return 0

quickMarchSpell = PythonModifier("sp-Quick March", 3, False) # spell_id, duration, empty
quickMarchSpell.AddHook(ET_OnConditionAdd, EK_NONE, quickMarchSpellCorrectDurationForCaster,())
quickMarchSpell.AddHook(ET_OnGetMoveSpeedBase, EK_NONE, quickMarchSpellMovementBonus,())
quickMarchSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
quickMarchSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
quickMarchSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
quickMarchSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
quickMarchSpell.AddSpellDispelCheckStandard()
quickMarchSpell.AddSpellTeleportPrepareStandard()
quickMarchSpell.AddSpellTeleportReconnectStandard()
quickMarchSpell.AddSpellCountdownStandardHook()
