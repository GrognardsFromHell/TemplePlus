from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import AoeSpellHandleModifier, AoESpellEffectModifier, getSpellHelpTag

print "Registering sp-Cloud of Bewilderment"

cloudOfBewildermentSpell = AoeSpellHandleModifier("sp-Cloud of Bewilderment") #spellId, duration, spellDc, eventId, empty

### Start Cloud Effect ###

def applyNauseatedCondition(attachee, args, evt_obj):
    if attachee.d20_query_has_condition("Nauseated"):
        duration = 1
        persistentFlag = 1
        attachee.d20_send_signal("PS_Nauseated_Update_Duration", duration, persistentFlag)
    else:
        spellDc = args.get_arg(2)
        spellId = args.get_arg(0)
        spellPacket = tpdp.SpellPacket(spellId)
        if not attachee.saving_throw_spell(spellDc, D20_Save_Fortitude, D20STD_F_NONE, spellPacket.caster, spellId): #save to avoid nauseated condition
            duration = 1
            persistentFlag = 1
            attachee.condition_add_with_args("Nauseated", duration, persistentFlag)
    return 0

def addConcealment(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    bonusValue = 20 #Considered concealed while in Cloud of Bewilderment
    bonusType = bonus_type_concealment
    bonusHelpTag = game.get_mesline("mes\\bonus_description.mes", bonusType)
    spellHelpTag = getSpellHelpTag(spellId)
    evt_obj.bonus_list.add(bonusValue, bonusType, "{} : {}".format(bonusHelpTag, spellHelpTag))
    return 0

def setNauseatedDurationOnLeave(attachee, args, evt_obj):
    #Nauseated effect lingers on after leaving the cloud or spell end for 1d4+1 rounds
    if attachee.d20_query_has_condition("Nauseated"):
        durationDice = dice_new('1d4')
        durationDice.bonus = 1
        duration = durationDice.roll()
        persistentFlag = 0
        attachee.d20_send_signal("PS_Nauseated_Update_Duration", duration, persistentFlag)
    return 0

cloudOfBewildermentCondition = AoESpellEffectModifier("Cloud of Bewilderment") #spellId, duration, spellDc, eventId, empty
cloudOfBewildermentCondition.AddHook(ET_OnConditionAdd, EK_NONE, applyNauseatedCondition, ())
cloudOfBewildermentCondition.AddHook(ET_OnBeginRound, EK_NONE, applyNauseatedCondition, ())
cloudOfBewildermentCondition.AddHook(ET_OnGetDefenderConcealmentMissChance, EK_NONE, addConcealment, ())
cloudOfBewildermentCondition.AddHook(ET_OnConditionRemove, EK_NONE, setNauseatedDurationOnLeave, ())
