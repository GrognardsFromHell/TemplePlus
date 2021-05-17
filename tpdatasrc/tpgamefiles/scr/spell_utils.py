from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *

### Standard Hooks invoked with AddHook ###

#[pytonModifier].AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
def spellTooltip(attachee, args, evt_obj):
    spellEnum = tpdp.SpellPacket(args.get_arg(0)).spell_enum
    spellName = game.get_spell_mesline(spellEnum)
    spellDuration = args.get_arg(1)
    if spellDuration == 1:
        evt_obj.append("{} ({} round)".format(spellName, spellDuration))
    else:
        evt_obj.append("{} ({} rounds)".format(spellName, spellDuration))
    return 0

#[pytonModifier].AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
def spellEffectTooltip(attachee, args, evt_obj):
    spellEnum = tpdp.SpellPacket(args.get_arg(0)).spell_enum
    spellName = game.get_spell_mesline(spellEnum)
    spellName = (spellName.upper()).replace(" ", "_")
    spellDuration = args.get_arg(1)
    if args.get_arg(1) == 1:
        evt_obj.append(tpdp.hash("{}".format(spellName)), -2, " ({} round)".format(spellDuration))
    else:
        evt_obj.append(tpdp.hash("{}".format(spellName)), -2, " ({} rounds)".format(spellDuration))
    return 0

#[pytonModifier].AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
def queryActiveSpell(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    if evt_obj.data1 == spellPacket.spell_enum:
        evt_obj.return_val = 1
    return 0

#[pytonModifier].AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
def spellKilled(attachee, args, evt_obj):
    args.remove_spell()
    args.remove_spell_mod()
    return 0

#[pytonModifier].AddHook(ET_OnD20Signal, EK_S_Spell_End, spell_utils.spellEnd, ())
def spellEnd(attachee, args, evt_obj):
    spellEnum = tpdp.SpellPacket(args.get_arg(0)).spell_enum
    spellName = game.get_spell_mesline(spellEnum)
    print "{} SpellEnd".format(spellName)
    return 0

#[pytonModifier].AddHook(ET_OnD20Signal, EK_S_Concentration_Broken, spell_utils.checkRemoveSpell, ())
#[pytonModifier].AddHook(ET_OnD20Signal, EK_S_Dismiss_Spells, spell_utils.checkRemoveSpell, ())
def checkRemoveSpell(attachee, args, evt_obj):
    if evt_obj.data1 == args.get_arg(0):
        args.remove_spell()
        args.remove_spell_mod()
    return 0

#[pytonModifier].AddHook(ET_OnConditionAdd, EK_NONE, spell_utils.addConcentration, ())
def addConcentration(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    spellPacket.caster.condition_add_with_args('sp-Concentrating', args.get_arg(0))
    return 0

#[pytonModifier].AddHook(ET_OnConditionAdd, EK_NONE, spell_utils.addDimiss, ())
def addDimiss(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    spellPacket.caster.condition_add_with_args('Dismiss', args.get_arg(0))
    return 0

#[pytonModifier].AddHook(ET_OnD20Signal, EK_S_Temporary_Hit_Points_Removed, spell_utils.removeTempHp, ())
# needed in combination with condition_add_with_args('Temporary_Hit_Points', 0, spell.duration, tempHpAmount)
def removeTempHp(attachee, args, evt_obj):
    attachee.d20_send_signal(S_Spell_End, 'Temporary_Hit_Points')
    return 0

### Other useful functions ###

# Skill Check with history windows #
def skillCheck(attachee, skillEnum, skillCheckDc):
    skillName = game.get_mesline("mes/skill.mes", skillEnum)
    bonusListSkill = tpdp.BonusList()
    skillValue = tpdp.dispatch_skill(attachee, skillEnum , bonusListSkill, OBJ_HANDLE_NULL, 1)
    skillDice = dice_new('1d20')
    skillDiceRoll = skillDice.roll()
    skillRollResult = skillDiceRoll + skillValue
    skillHistoryId = tpdp.create_history_dc_roll(attachee, skillCheckDc, skillDice, skillDiceRoll, "{}".format(skillName), bonusListSkill)
    game.create_history_from_id(skillHistoryId)
    checkResult = True if skillRollResult >= skillCheckDc else False
    return checkResult

