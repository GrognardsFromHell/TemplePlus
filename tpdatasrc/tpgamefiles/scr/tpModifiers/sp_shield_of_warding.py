from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Shield of Warding"

def shieldOfWardingSpellAddShieldCondition(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    spellPacket.add_target(attachee, 0)
    attachee.item_condition_add_with_args('Shield of Warding Condition', args.get_arg(2), 0, 0, 0, args.get_arg(0))
    return 0

def shieldOfWardingSpellRemoveShieldCondition(attachee, args, evt_obj):
    attachee.item_condition_remove('Shield of Warding Condition', args.get_arg(0))
    return 0

shieldOfWardingSpell = PythonModifier("sp-Shield of Warding", 4) # spell_id, duration, bonusValue, empty
shieldOfWardingSpell.AddHook(ET_OnConditionAdd, EK_NONE, shieldOfWardingSpellAddShieldCondition,())
shieldOfWardingSpell.AddHook(ET_OnConditionRemove, EK_NONE, shieldOfWardingSpellRemoveShieldCondition, ())
shieldOfWardingSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
shieldOfWardingSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
shieldOfWardingSpell.AddSpellDispelCheckStandard()
shieldOfWardingSpell.AddSpellTeleportPrepareStandard()
shieldOfWardingSpell.AddSpellTeleportReconnectStandard()
shieldOfWardingSpell.AddSpellCountdownStandardHook()

###### Shield of Warding Condition ######
def shieldOfWardingConditionBonus(attachee, args, evt_obj):
    if args.get_arg(2) == 211: #Shield needs to be equipped to have spell effects work
        #Shield of Warding grants a bonus to AC and Reflex save; value(arg2) is passed by spell
        bonusValue = args.get_arg(0)
        bonusType = 153 #ID 153 Sacred; This might be wrong!
        evt_obj.bonus_list.add(bonusValue, bonusType, "~Sacred~[TAG_MODIFIER_SACRED] : ~Shield of Warding~[TAG_SPELLS_SHIELD_OF_WARDING]")

shieldOfWardingCondition = PythonModifier("Shield of Warding Condition", 5) # bonusValue, empty, inventoryLocation, empty, spell_id
shieldOfWardingCondition.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, shieldOfWardingConditionBonus, ())
shieldOfWardingCondition.AddHook(ET_OnGetAC, EK_NONE, shieldOfWardingConditionBonus, ())
