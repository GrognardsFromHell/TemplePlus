from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Marshal Adrenaline Boost"

def onConditionPreActions(attachee, args, evt_obj):
    #Adrenaline Boost does not stack with itself.
    #If used again while old Boost is active
    #tempHP will be reset; this is done by removing
    #the old condition and apply the newer one
    if evt_obj.is_modifier("sp-Marshal Adrenaline Boost"):
        evt_obj.return_val = 1
        args.condition_remove()
    return 0

def addTempHP(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    duration = args.get_arg(1)
    currentHp = attachee.stat_level_get(stat_hp_current)
    maxHp = attachee.stat_level_get(stat_hp_max)
    tempHp = spellPacket.caster_level
    #Adrenaline Boost TempHP are doubled if target is at or below half HP
    if (currentHp * 2) <= maxHp:
        tempHp *= 2
    attachee.condition_add_with_args('Temporary_Hit_Points', spellId, duration, tempHp)
    return 0

def removeTempHp(attachee, args, evt_obj):
    attachee.d20_send_signal(S_Spell_End, args.get_arg(0))
    return 0

marshalAdrenalineBoostSpell = PythonModifier("sp-Marshal Adrenaline Boost", 4, False) # spell_id, duration, empty, empty
marshalAdrenalineBoostSpell.AddHook(ET_OnConditionAddPre, EK_NONE, onConditionPreActions,())
marshalAdrenalineBoostSpell.AddHook(ET_OnConditionAdd, EK_NONE, addTempHP,())
marshalAdrenalineBoostSpell.AddHook(ET_OnConditionRemove, EK_NONE, removeTempHp, ())
marshalAdrenalineBoostSpell.AddHook(ET_OnD20Signal, EK_S_Temporary_Hit_Points_Removed, spell_utils.removeTempHp, ())
marshalAdrenalineBoostSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
marshalAdrenalineBoostSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
marshalAdrenalineBoostSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
marshalAdrenalineBoostSpell.AddSpellTeleportPrepareStandard()
marshalAdrenalineBoostSpell.AddSpellTeleportReconnectStandard()
marshalAdrenalineBoostSpell.AddSpellCountdownStandardHook()

