from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *

# Get the spell's name from its spell_id
def spellName(spellId):
    spellEnum = tpdp.SpellPacket(spellId).spell_enum
    return game.get_spell_mesline(spellEnum)

def spellKeyName(spellId):
    return spellName(spellId).upper().replace(" ", "_")

def spellTag(spellId):
    return "TAG_SPELLS_{}".format(spellKeyName(spellId))

def spellKey(spellId):
    return tpdp.hash(spellKeyName(spellId))

def getSpellHelpTag(spellId):
    return "~{}~[{}]".format(spellName(spellId), spellTag(spellId))

def spellTime(duration):
    if duration == 1:
        return "1 round"
    elif duration < 100: # 10 minutes
        return "{} rounds".format(duration)
    elif duration < 1200: # 2 hours
        return "{} minutes".format(duration/10)
    elif duration < 28800: # 2 days
        return "{} hours".format(duration/600)
    elif duration < 864000: # 2 months
        return "{} days".format(duration/14400)
    else:
        return "{} months".format(duration/432000)

### Standard Hooks invoked with AddHook ###

# Handles tooltip requests for a condition. Can be installed as:
#
# [pytonModifier].AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ()) or
# [pytonModifier].AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, (spell_enum,))
#
# The second one is needed, if tooltip of a different spell should be shown
# e.g. single target version when mass version is cast
#
# Optionally, 1 can be passed as a 2nd parameter. If 1 is passed
# then an appropriate duration will be reported
# for spells that only start counting down
# after the caster's concentration is broken:
#
# [pytonModifier].AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, (0, 1))
def spellTooltip(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    duration = spellTime(args.get_arg(1))
    if args.get_param(1) == 1 and casterIsConcentrating(spellId):
        name = spellName(spellId)
        duration = "Concentration + {}".format(duration)
    elif args.get_param(0):
        name = game.get_spell_mesline(args.get_param(0))
    else:
        name = spellName(spellId)
    evt_obj.append("{} ({})".format(name, duration))
    return 0

# Handles effect tooltip (the tooltip of the icon on the portrait)
# requests for a condition. Can be installed as:
#
# [pytonModifier].AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ()) or
# [pytonModifier].AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, (spell_enum,))
# The second one is needed, if Effect Tooltip of a different spell should be shown
# e.g. single target version when mass version is cast
#
# Optionally, 1 can be passed as a 2nd parameter. If 1 is passed,
# then an appropriate duration will be reported
# for spells that only start counting down
# after the caster's concentration is broken:
#
# [pytonModifier].AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, (0, 1))
def spellEffectTooltip(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    duration = spellTime(args.get_arg(1))
    if args.get_param(1) == 1 and casterIsConcentrating(spellId):
        duration = "Concentration + {}".format(duration)
        key = spellKey(spellId)
    elif args.get_param(0):
        name = game.get_spell_mesline(args.get_param(0)).upper().replace(" ", "_")
        key = tpdp.hash(name)
    else:
        key = spellKey(spellId)
    evt_obj.append(key, -2, " ({})".format(duration))
    return 0

#[pytonModifier].AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
def queryActiveSpell(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    if evt_obj.data1 == spellPacket.spell_enum:
        evt_obj.return_val = 1
    return 0

def querySpellCondition(attachee, args, evt_obj):
    queryConditionRef = evt_obj.data1
    conditionName = args.get_cond_name()
    spellConditionRef = tpdp.get_condition_ref(conditionName)
    if queryConditionRef == spellConditionRef:
        evt_obj.return_val = 1
    return 0

#[pytonModifier].AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
def spellKilled(attachee, args, evt_obj):
    args.remove_spell()
    args.remove_spell_mod()
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

#[pytonModifier].AddHook(ET_OnConditionAdd, EK_NONE, spell_utils.addDismiss, ())
def addDismiss(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    spellPacket.caster.condition_add_with_args('Dismiss', args.get_arg(0))
    return 0

#[pytonModifier].AddHook(ET_OnD20Signal, EK_S_Temporary_Hit_Points_Removed, spell_utils.removeTempHp, ())
# needed in combination with condition_add_with_args('Temporary_Hit_Points', spell.id, spell.duration, tempHpAmount)
def removeTempHp(attachee, args, evt_obj):
    attachee.d20_send_signal(S_Spell_End, args.get_arg(0))
    return 0

def casterIsConcentrating(spellId):
    packet = tpdp.SpellPacket(spellId)

    caster = packet.caster
    if caster.d20_query(Q_Critter_Is_Concentrating):
        conId = caster.d20_query_get_data(Q_Critter_Is_Concentrating)
        return spellId == conId
    else: return False

# Some spells have a duration like:
#
#   Concentration + 2 rounds
#   Concentration + 1 round/level
#
# This function implements that behavior when installed as a
# hook for ET_OnBeginRound. It decrements the condition's duration
# only after concentration is broken by the caster.
def countAfterConcentration(attachee, args, evt_obj):
    if casterIsConcentrating(args.get_arg(0)): return 0

    newDur = args.get_arg(1) - evt_obj.data1
    if newDur >= 0:
        args.set_arg(1, newDur)
        return 0

    args.remove_spell_with_key(EK_S_Concentration_Broken)

    return 0

#Used to replace same condition to prevent duplicates
def replaceCondition(attachee, args, evt_obj):
    conditionName = args.get_cond_name()
    if evt_obj.is_modifier(conditionName):
        args.remove_spell()
        args.remove_spell_mod()
    return 0

# Used to check if a condition should be removed due to a new 'save'
# from Countersong. Each round of a countersong, the bard makes a
# perform check, and if it is equal to or greater than the spell's
# save DC, the spell ends as if saved against.
#
# Note: only spells with ongoing duration should be able to be removed
# by countersong. For example, Sound Burst and Shout are instantaneous
# sonic effects that can cause conditions with a duration, so those
# conditions cannot be removed by countersong.  Another bard's
# fascinating song may be countered, though.
#
# Usage:
#   cond.AddHook(ET_OnConditionAddPre, EK_NONE, countersongRemove, ())
def countersongRemove(attachee, args, evt_obj):
    if not evt_obj.is_modifier('Countersong'): return 0

    spellId = args.get_arg(0)
    if spellId <= 0: return 0

    packet = tpdp.SpellPacket(spellId)

    # check for descriptors in the spell entry in case conditions are
    # being reused
    entry = tpdp.SpellEntry(packet.spell_enum)
    sonic = 1 << (D20STD_F_SPELL_DESCRIPTOR_SONIC-1)
    lang = 1 << (D20STD_F_SPELL_DESCRIPTOR_LANGUAGE_DEPENDENT-1)
    mask = sonic|lang
    if not (entry.descriptor & mask): return 0

    if packet.dc <= evt_obj.arg2:
        args.remove_spell()
        args.remove_spell_mod()

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

# Check mc_type
def checkCategoryType(critter, *args):
    for mcType in args:
        if critter.is_category_type(mcType):
            return True
    return False

##### workaround getSpellClassCode #####
def getSpellClassCode(classEnum):
    dummySpellData = tpdp.D20SpellData()
    dummySpellData.set_spell_class(classEnum)
    return dummySpellData.spell_class
##### workaround getSpellClassCode #####

### Item Condition functions

# An item condition is a condition that should be applied to a
# character when they hold/wear an item (hold/wear depending on the
# item). Callbacks should be written as if the condition is applied to
# the character, even though the condition is initially applied to the
# item.
#
# These conditions should have at least 5 arguments, and argument #2
# is automatically set by the engine to the inventory location of the
# item providing the condition. By convention argument #4 is the spell
# id if the item condition is added by a spell.

# Verifies an item object against the engine-set inventory location
# for the effect.
def verifyItem(item, args):
    item_loc = item.obj_get_int(obj_f_item_inv_location)
    target_loc = args.get_arg(2)

    return item_loc == target_loc

# getItemObj, itemTooltip and itemEffectTooltip are needed for
# buff tooltip icon handling
def getItemObj(attachee, args):
    itemLocation = args.get_arg(2)
    if itemLocation == 203: #mainhand
        item = attachee.item_worn_at(item_wear_weapon_primary)
        itemWornAt = "mainhand"
    elif itemLocation == 204: #offhand
        item = attachee.item_worn_at(item_wear_weapon_secondary)
        itemWornAt = "offhand"
    elif itemLocation == 205: #armor
        item = attachee.item_worn_at(item_wear_armor)
        itemWornAt = ""
    elif itemLocation == 211: #shield
        item = attachee.item_worn_at(item_wear_shield)
        itemWornAt = ""
    return item, itemWornAt

def itemTooltip(attachee, args, evt_obj):
    item, itemWornAt = getItemObj(attachee, args)
    spellId = args.get_arg(4)
    durationQuery = item.d20_query_with_data("PQ_Item_Buff_Duration", spellId, 0)
    if durationQuery:
        duration = spellTime(durationQuery)
        if args.get_param(0):
            name = game.get_spell_mesline(args.get_param(0))
        else:
            name = spellName(spellId)
        evt_obj.append("{}({}) ({})".format(name, itemWornAt, duration))
        return 0

def itemEffectTooltip(attachee, args, evt_obj):
    item, itemWornAt = getItemObj(attachee, args)
    spellId = args.get_arg(4)
    durationQuery = item.d20_query_with_data("PQ_Item_Buff_Duration", spellId, 0)
    if durationQuery:
        duration = spellTime(durationQuery)
        if args.get_param(0):
            name = game.get_spell_mesline(args.get_param(0)).upper().replace(" ", "_")
            key = tpdp.hash(name)
        else:
            key = spellKey(spellId)
        evt_obj.append(key, -2, "({}) ({})".format(itemWornAt, duration))
    return 0

### Utilities for defining touch attacks with held charge ###

# Keys off 'SPELL_NAME_CHARGE' so that a buff indicator for the holding
# charge state can be separated from a debuff indicator for the spell's
# effect.
def touchKey(spellId):
    return tpdp.hash("{}_CHARGE".format(spellKeyName(spellId)))

# Presumes that duration and charges are as the helper class below
def touchInfo(args):
    duration = args.get_arg(1)
    charges = args.get_arg(2)

    msgs = []
    if charges == 1:
        msgs.append("1 charge")
    elif charges > 1:
        msgs.append("{} charges".format(charges))

    if duration > 0:
        msgs.append(spellTime(duration))

    paren = ""
    if len(msgs) > 0:
        paren = " ({})".format(", ".join(msgs))

    return paren

# Handles the tooltip event for held touch spells.
def touchTooltip(attachee, args, evt_obj):
    evt_obj.append("{}{}".format(spellName(args.get_arg(0)), touchInfo(args)))
    return 0

# Handles the effect tooltip event for held touch spells.
def touchEffectTooltip(attachee, args, evt_obj):
    evt_obj.append(touchKey(args.get_arg(0)), -2, touchInfo(args))
    return 0

# Indicate which spell a character with this condition
# is holding a charge for.
def touchHoldingCharge(attachee, args, evt_obj):
    spell_id = args.get_arg(0)

    evt_obj.return_val = 1
    evt_obj.data1 = spell_id
    return 0

# if a charge attack is added, and it's not the spell _this_
# condition was added from, the spell needs to end.
def touchTouchAttackAdded(attachee, args, evt_obj):
    spell_id = args.get_arg(0)

    if evt_obj.data1 != spell_id:
        args.condition_remove()
        args.remove_spell()
    return 0

# On the round a charge is held, a free touch attack may be
# made, since it is supposed to happen as part of casting.
def touchAdd(attachee, args, evt_obj):
    spell_id = args.get_arg(0)

    tbFlags = tpdp.cur_seq_get_turn_based_status_flags()
    tbFlags = tbFlags | TBSF_TouchAttack
    tpdp.cur_seq_set_turn_based_status_flags(tbFlags)
    attachee.d20_send_signal(S_TouchAttackAdded, spell_id)
    return 0

# End the spell and remove the holding-charge condition.
def End(attachee, args, evt_obj):
    args.condition_remove()
    args.remove_spell()
    return 0

# End the spell if another spell is cast by the condition
# target. The callback is invoked for targets of spells as
# well, so it's necessary to check the spell packet to see
# if the caster is the same as the attachee.
def touchOtherCast(attachee, args, evt_obj):
    spell_id = evt_obj.data1
    packet = tpdp.SpellPacket(spell_id)
    if packet.caster == attachee:
        End(attachee, args, evt_obj)
    return 0

# Common code for handling touch attack charges.
#
# 1) Decrements number of charges if positive
# 2) Ends the condition if charges fall to 0
# 3) Checks spell resistance on the target.
# 4) If resistance fails, adds the target to the
#        spell's target list
#
# Returns whether the spell was resisted so that
# further effects can be run.
#
# Common use would be something like:
#
#  def OnTouch(attachee, args, evt_obj):
#    resisted = touchPre(args, evt_obj)
#
#    if not resisted:
#      <spell effects>
def touchPre(args, evt_obj):
    action = evt_obj.get_d20_action()
    caster = action.performer
    target = action.target

    spell_id = args.get_arg(0)
    duration = args.get_arg(1)
    charges = args.get_arg(2)

    packet = tpdp.SpellPacket(spell_id)

    if charges > 0:
        args.set_arg(2, charges-1)

    resisted = packet.check_spell_resistance(target)

    if charges == 1:
        packet.end_target_particles(caster)
        packet.remove_target(caster)
        args.condition_remove()

    if resisted:
        game.particles('Fizzle', target)
        game.sound(7461,1)
    else:
        packet.add_target(target, 0)

    packet.update_registry()
    if packet.target_count <= 0:
        args.spell_remove()

    return resisted

# Modifiers representing a 'held charge' touch spell
class TouchModifier(PythonModifier):
    # Touch modifiers have at least 3 arguments
    #
    #  0: spell_id
    #  1: duration, negative to not display
    #  2: num_charges, negative for until duration expires
    #
    # the given argument number is for additional arguments beyond
    # this.
    #
    # The standard countdown hook is not added, since the typical
    # mechanic is for charges to be held indefinitely until used.
    # An argument is reserved for time limited touch spells,
    # however, and the countdown hook can be added in such a case.
    def __init__(self, name, argn):
        PythonModifier.__init__(self, name, 3+argn, 0)

        self.AddHook(ET_OnGetTooltip, EK_NONE, touchTooltip, ())
        self.AddHook(ET_OnGetEffectTooltip, EK_NONE, touchEffectTooltip, ())

        # Handle queries for whether the affected creature is
        # 'holding charge'?
        self.AddHook(ET_OnD20Query, EK_Q_HoldingCharge, touchHoldingCharge, ())

        # get notifications of other charge attacks being added
        self.AddHook(
                ET_OnD20Signal, EK_S_TouchAttackAdded,
                touchTouchAttackAdded, ())

        # casting another spell ends held charge spells
        self.AddHook(ET_OnD20Signal, EK_S_Spell_Cast, touchOtherCast, ())

        # modify the menus to allow for touch attacks
        self.AddHook(ET_OnConditionAdd, EK_NONE, touchAdd, ())

        self.AddHook(ET_OnNewDay, EK_NEWDAY_REST, End, ())

        self.AddSpellDispellCheckHook()
        self.AddSpellTeleportPrepareStandard()
        self.AddSpellTeleportReconnectStandard()
        self.AddSpellTouchAttackDischargeRadialMenuHook()

    def AddTouchHook(self, hook):
        self.AddHook(ET_OnD20Signal, EK_S_TouchAttack, hook, ())

######## Spell Python Modifier classes ########

### Standard Spell Condition Modifier ###

def getBonusHelpTag(bonusType):
    return game.get_mesline("mes\\bonus_description.mes", bonusType)

def applyBonus(attachee, args, evt_obj):
    bonusValue = args.get_param(0)
    if not bonusValue:
        bonusValue = args.get_arg(2)
    bonusType = args.get_param(1)
    bonusHelpTag = getBonusHelpTag(bonusType)
    spellId = args.get_arg(0)
    spellHelpTag = getSpellHelpTag(spellId)
    evt_obj.bonus_list.add(bonusValue, bonusType, "{} : {}".format(bonusHelpTag, spellHelpTag))
    return 0

def applyDamageReduction(attachee, args, evt_obj):
    drAmount = args.get_param(0)
    drBreakType = args.get_param(1)
    damageMesId = 126 #ID 126 = ~Damage Reduction~[TAG_SPECIAL_ABILITIES_DAMAGE_REDUCTION]
    evt_obj.damage_packet.add_physical_damage_res(drAmount, drBreakType, damageMesId)
    return 0

def applySaveBonus(attachee, args, evt_obj):
    saveDescriptor = args.get_param(2)
    if saveDescriptor:
        if not evt_obj.flags & (1 << (saveDescriptor - 1)):
            return 0
    applyBonus(attachee, args, evt_obj)
    return 0

def applyAttackPacketBonus(attachee, args, evt_obj):
    flagRequirement = args.get_param(2)
    if flagRequirement:
        if not evt_obj.attack_packet.get_flags() & flagRequirement:
            return 0
    applyBonus(attachee, args, evt_obj)
    return 0

class SpellPythonModifier(PythonModifier):
    #SpellPythonModifier have at least 3 arguments:
    #spellId, duration, empty
    #if a simple bonusValue is passed by the spell it is set to arg 3:
    #spellId, duration, bonusValue, empty
    def __init__(self, name, args = 3, preventDuplicate = False):
        PythonModifier.__init__(self, name, args, preventDuplicate)
        self.AddHook(ET_OnGetTooltip, EK_NONE, spellTooltip, ())
        self.AddHook(ET_OnGetEffectTooltip, EK_NONE, spellEffectTooltip, ())
        self.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, queryActiveSpell, ())
        self.AddHook(ET_OnD20Signal, EK_S_Killed, spellKilled, ())
        self.AddSpellDispelCheckStandard()
        self.AddSpellCountdownStandardHook()
        self.AddSpellTeleportPrepareStandard()
        self.AddSpellTeleportReconnectStandard()
    def AddSpellConcentration(self):
        self.AddHook(ET_OnConditionAdd, EK_NONE, addConcentration, ())
        self.AddHook(ET_OnD20Signal, EK_S_Concentration_Broken, checkRemoveSpell, ())
    def AddSpellDismiss(self):
        self.AddHook(ET_OnConditionAdd, EK_NONE, addDismiss, ())
        self.AddHook(ET_OnD20Signal, EK_S_Dismiss_Spells, checkRemoveSpell, ())
    def AddSpellNoDuplicate(self):
        self.AddHook(ET_OnConditionAddPre, EK_NONE, replaceCondition, ())
    def AddSkillBonus(self, bonusValue, bonusType, *args):
        if not args:
            eventKey = EK_NONE
            self.AddHook(ET_OnGetSkillLevel, eventKey, applyBonus, (bonusValue, bonusType,))
        else:
            for skill in args:
                eventKey = skill + 20
                self.AddHook(ET_OnGetSkillLevel, eventKey, applyBonus, (bonusValue, bonusType,))
    def AddAbilityBonus(self, bonusValue, bonusType, *args):
        for abilityScore in args:
            eventKey = abilityScore + 1
            self.AddHook(ET_OnAbilityScoreLevel, eventKey, applyBonus,(bonusValue, bonusType,))
    def AddDamageReduction(self, drAmount, drBreakType):
        self.AddHook(ET_OnTakingDamage2, EK_NONE, applyDamageReduction,(drAmount, drBreakType,))
    def AddSaveBonus(self, bonusValue, bonusType, eventKey = EK_NONE, saveDescriptor = D20STD_F_NONE):
        self.AddHook(ET_OnSaveThrowLevel, eventKey, applySaveBonus, (bonusValue, bonusType, saveDescriptor,))
    def AddToHitBonus(self, bonusValue, bonusType, flagRequirement = 0):
        self.AddHook(ET_OnToHitBonus2, EK_NONE, applyAttackPacketBonus,(bonusValue, bonusType, flagRequirement,))
    def AddAcBonus(self, bonusValue, bonusType):
        self.AddHook(ET_OnGetAC, EK_NONE, applyBonus, (bonusValue, bonusType,))
    def AddAbilityCheckBonus(self, bonusValue, bonusType): #might get expanded
        self.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE, applyBonus, (bonusValue, bonusType,))
    def AddMovementBonus(self, bonusValue, bonusType):
        self.AddHook(ET_OnGetMoveSpeedBase, EK_NONE, applyBonus, (bonusValue, bonusType,))

### Aoe Modifier Classes ###
def addAoeObjToSpellRegistry(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    conditionName = args.get_cond_name()
    particlesId = game.particles(conditionName, attachee)
    spellPacket.add_spell_object(attachee, particlesId)
    spellPacket.update_registry()
    return 0

def aoeOnEnter(attachee, args, evt_obj):
    print "aoeOnEnter Hook"
    spellId = args.get_arg(0)
    duration = args.get_arg(1)
    spellDc = args.get_arg(2)
    spellEventId = args.get_arg(3)
    spellPacket = tpdp.SpellPacket(spellId)
    spellTarget = evt_obj.target
    print "spellTarget: {}".format(spellTarget)

    if spellEventId != evt_obj.evt_id:
        return 0
    elif spellTarget == OBJ_HANDLE_NULL:
        return 0
    elif attachee == OBJ_HANDLE_NULL:
        return 0
    #elif spellTarget == attachee:
    #    return 0
    elif spellPacket.check_spell_resistance(spellTarget):
        return 0

    spellPacket.trigger_aoe_hit()

    conditionName = args.get_cond_name()
    particlesId = game.particles("{}-hit".format(conditionName), spellTarget)

    if spellPacket.add_target(spellTarget, particlesId):
        conditionEffectName = spellName(spellId)
        spellTarget.condition_add_with_args(conditionEffectName, spellId, duration + 1, spellDc, spellEventId)
        spellPacket.update_registry()
    return 0

def aoeHandleEndSignal(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    if evt_obj.data1 == spellId:
        spellPacket = tpdp.SpellPacket(spellId)
        #spellMesId = 20000 # ID 20000 = A spell has expired.
        if spellPacket.spell_enum == 0:
            return 0
        spellTargetCount = 0
        while spellTargetCount < spellPacket.target_count:
            spellTarget = spellPacket.get_target(spellTargetCount)
            spellTarget.d20_send_signal(S_Spell_End, spellId)
            #spellTarget.float_mesfile_line('mes\\spell.mes', spellMesId)
            spellTargetCount += 1
    return 0

def aoeCombatEndSignal(attachee, args, evt_obj):
    args.set_arg(1, -1)
    return 0

def aoeOnLeave(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    spellEventId = args.get_arg(3)

    if spellEventId != evt_obj.evt_id:
        return 0

    spellPacket.end_target_particles(attachee)
    spellPacket.remove_target(attachee)
    args.remove_spell_mod()
    return 0

def aoeSpellEndSignal(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    if spellId == evt_obj.data1:
        spellPacket = tpdp.SpellPacket(spellId)
        spellPacket.end_target_particles(attachee)
        spellPacket.remove_target(attachee)
        spellPacket.update_registry()
        args.remove_spell_mod()
    return 0

def aoeEffectTickdown(attachee, args, evt_obj):
    #This is only done, so the Tooltip can show the duration
    #Actual spell end is handled by the AoeSpellHandlerModifier
    duration = args.get_arg(1)
    duration -= evt_obj.data1
    args.set_arg(1, duration)
    return 0

class AoeSpellHandleModifier(PythonModifier):
    #AoeSpellHandlerModifier have at least 5 arguments:
    #spellId, duration, spellDc, spellEventId, empty
    def __init__(self, name, args = 5, preventDuplicate = False):
        PythonModifier.__init__(self, name, args, preventDuplicate)
        self.AddHook(ET_OnConditionAdd, EK_NONE, addAoeObjToSpellRegistry, ())
        self.AddHook(ET_OnObjectEvent, EK_OnEnterAoE, aoeOnEnter, ())
        self.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, queryActiveSpell, ())
        self.AddHook(ET_OnD20Signal, EK_S_Combat_End, aoeCombatEndSignal, ())
        self.AddSpellDispelCheckStandard()
        self.AddSpellTeleportPrepareStandard()
        self.AddSpellTeleportReconnectStandard()
        self.AddSpellCountdownStandardHook()
        self.AddAoESpellEndStandardHook()
    def AddSpellConcentration(self):
        self.AddHook(ET_OnConditionAdd, EK_NONE, addConcentration, ())
        self.AddHook(ET_OnD20Signal, EK_S_Concentration_Broken, aoeHandleEndSignal, ())
    def AddSpellDismiss(self):
        self.AddHook(ET_OnConditionAdd, EK_NONE, addDismiss, ())
        self.AddHook(ET_OnD20Signal, EK_S_Dismiss_Spells, aoeHandleEndSignal, ())
    def AddSpellNoDuplicate(self):
        self.AddHook(ET_OnConditionAddPre, EK_NONE, replaceCondition, ())

class AoESpellEffectModifier(PythonModifier):
    #AoESpellEffectPythonModifier have at least 5 arguments:
    #spellId, duration, spellDc, spellEventId, empty
    def __init__(self, name, args = 5, preventDuplicate = True):
        PythonModifier.__init__(self, name, args, preventDuplicate)
        #self.AddHook(ET_OnBeginRound, EK_NONE, aoeEffectTickdown, ())
        self.AddHook(ET_OnObjectEvent, EK_OnLeaveAoE, aoeOnLeave, ())
        self.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, queryActiveSpell, ())
        self.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Condition, querySpellCondition, ())
        self.AddHook(ET_OnGetTooltip, EK_NONE, spellTooltip, ())
        self.AddHook(ET_OnGetEffectTooltip, EK_NONE, spellEffectTooltip, ())
        self.AddHook(ET_OnD20Signal, EK_S_Spell_End, aoeSpellEndSignal, ())
        self.AddHook(ET_OnD20Signal, EK_S_Combat_End, aoeCombatEndSignal, ())
        self.AddHook(ET_OnD20Signal, EK_S_Killed, spellKilled, ())
        self.AddSpellCountdownStandardHook()
        self.AddSpellDispellCheckHook()
        self.AddSpellTeleportPrepareStandard()
        self.AddSpellTeleportReconnectStandard()
