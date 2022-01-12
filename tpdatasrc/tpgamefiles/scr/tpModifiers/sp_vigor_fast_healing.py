from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Vigor Fast Healing"

def healTick(attachee, args, evt_obj):
    healAmount = args.get_arg(2)
    spellId = args.get_arg(0)
    spellEnum = spellEnum = tpdp.SpellPacket(spellId).spell_enum
    spellName = game.get_spell_mesline(spellEnum)
    spellTag = "TAG_SPELLS_{}".format(spellName.upper().replace(" ", "_"))
    ### workaround for heal ###
    #heal requires a dice
    healDice = dice_new('1d1')
    healDice.bonus = healAmount -1
    ### workaround end ###
    game.particles ('sp-Vigor', attachee)
    if attachee.obj_get_int(obj_f_critter_subdual_damage):
        attachee.healsubdual(attachee, healDice, D20A_HEAL, 0)
    elif attachee.obj_get_int(obj_f_hp_damage):
        attachee.heal(attachee, healDice, D20A_HEAL, 0)
    if game.combat_is_active(): #Limit History output to combat only
        game.create_history_freeform("{} is healed for {} by ~{}~[{}]\n\n".format(attachee.description, healAmount, spellName, spellTag))
    return 0

### Workaround ###
def getHealAmount(spellEnum):
    if spellEnum == spell_vigor_lesser or spell_vigor_mass_lesser:
        return 1
    elif spellEnum == spell_vigor:
        return 2
    elif spellEnum == spell_vigor_greater:
        return 4
    elif spellEnum == spell_vigor_mass_improved:
        return 3
    return 0
### Workaround End ###

def onConditionAddPreActions(attachee, args, evt_obj):
    conditionName = args.get_cond_name()
    if evt_obj.is_modifier(conditionName):
        ### Workaround as EventObjModifier doesn't grant access to arg3 ###
        newCondSpellId = evt_obj.arg1
        newCondSpellEnum = tpdp.SpellPacket(newCondSpellId).spell_enum
        newCondHealAmount = getHealAmount(newCondSpellEnum)
        ### Workaround End ###
        healAmount = args.get_arg(2)
        if newCondHealAmount >= healAmount:
            args.remove_spell()
            args.remove_spell_mod()
        else:
            evt_obj.return_val = 0
    return 0

vigorFastHealing = SpellPythonModifier("sp-Vigor Fast Healing", 4) # spellId, duration, healAmount, empty
vigorFastHealing.AddHook(ET_OnBeginRound, EK_NONE, healTick,())
vigorFastHealing.AddHook(ET_OnConditionAddPre, EK_NONE, onConditionAddPreActions, ())
