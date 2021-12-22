from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Improvisation"

#Args:
# 0 spell_id
# 1 duration
# 2 bonusValue
# 3 bonusPool
# 4 activateAbility
# 5 activateSkill
# 6 activateAttack

def improvisationSpellConditionAdd(attachee, args, evt_obj):
    attachee.float_text_line("Luck Pool: {}".format(args.get_arg(3)))
    return 0

def improvisationSpellRadial(attachee, args, evt_obj):
    bonusValue = args.get_arg(2)
    bonusPool = args.get_arg(3)
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    improvisationBonusCap = min(bonusPool, spellPacket.caster_level/2) #Bonus cannot be higher than points left in BonusPool

    #Add the top level menu
    radialParent = tpdp.RadialMenuEntryParent("Improvisation ({})".format(args.get_arg(3)))
    improvisationRadialId = radialParent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)

    #Add a slider radial to set the bonus
    sliderSetBonusAmount = tpdp.RadialMenuEntrySlider("Set Bonus Amount", "Set Bonus Amount for Improvisation", 0, improvisationBonusCap, "TAG_INTERFACE_HELP")
    sliderSetBonusAmount.link_to_args(args, 2)
    sliderSetBonusAmount.add_as_child(attachee, improvisationRadialId)

    #Add toggle radials to activate or deactivate the bonus for different options
    toggleAbilityBonus = tpdp.RadialMenuEntryToggle("+{} to next Ability Check".format(bonusValue), "TAG_SPELLS_IMPROVISATION")
    toggleAbilityBonus.link_to_args(args, 4)
    toggleAbilityBonus.add_as_child(attachee, improvisationRadialId)

    toggleSkillBonus = tpdp.RadialMenuEntryToggle("+{} to next Skill Check".format(bonusValue), "TAG_SPELLS_IMPROVISATION")
    toggleSkillBonus.link_to_args(args, 5)
    toggleSkillBonus.add_as_child(attachee, improvisationRadialId)

    toggleAttackBonus = tpdp.RadialMenuEntryToggle("+{} to next Attack".format(bonusValue), "TAG_SPELLS_IMPROVISATION")
    toggleAttackBonus.link_to_args(args, 6)
    toggleAttackBonus.add_as_child(attachee, improvisationRadialId)
    return 0

def reducePool(attachee, args):
    args.set_arg(3, args.get_arg(3)-args.get_arg(2))
    if args.get_arg(3) > 0:
        attachee.float_text_line("Luck Pool Left: {}".format(args.get_arg(3)))
        if args.get_arg(3) < args.get_arg(2):
            args.set_arg(2, args.get_arg(3))
    else:
        attachee.float_text_line("Luck Pool depleted")
        args.remove_spell()
        args.remove_spell_mod()
    return 0

def applyBonus(args, evt_obj):
    bonusValue = args.get_arg(2)
    bonusType = 14 #ID 14 = Luck Bonus
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Improvisation~[TAG_SPELLS_IMPROVISATION] ~Luck~[TAG_MODIFIER_LUCK] Bonus")

def improvisationSpellAbilityCheckBonus(attachee, args, evt_obj):
    if args.get_arg(4): #check if enabled
        applyBonus(args, evt_obj)
        reducePool(attachee, args)
    return 0

def improvisationSpellSkillCheckBonus(attachee, args, evt_obj):
    if args.get_arg(5): #check if enabled
        applyBonus(args, evt_obj)
        reducePool(attachee, args)
    return 0

def improvisationSpellAttackBonus(attachee, args, evt_obj):
    if not (evt_obj.attack_packet.get_flags() & D20CAF_FINAL_ATTACK_ROLL): #Test to make sure it is not called from the character sheet
        return 0
    if args.get_arg(6): #check if enabled
        applyBonus(args, evt_obj)
        reducePool(attachee, args)
    return 0

improvisationSpell = PythonModifier("sp-Improvisation", 7) # spell_id, duration, bonusValue, bonusPool, activateAbility, activateSkill, activateAttack
improvisationSpell.AddHook(ET_OnConditionAdd, EK_NONE, improvisationSpellConditionAdd, ())
improvisationSpell.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, improvisationSpellRadial, ())
improvisationSpell.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE, improvisationSpellAbilityCheckBonus,())
improvisationSpell.AddHook(ET_OnGetSkillLevel, EK_NONE, improvisationSpellSkillCheckBonus, ())
improvisationSpell.AddHook(ET_OnToHitBonus2, EK_NONE, improvisationSpellAttackBonus, ())
improvisationSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
improvisationSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
improvisationSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
improvisationSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
improvisationSpell.AddSpellDispelCheckStandard()
improvisationSpell.AddSpellTeleportPrepareStandard()
improvisationSpell.AddSpellTeleportReconnectStandard()
improvisationSpell.AddSpellCountdownStandardHook()
