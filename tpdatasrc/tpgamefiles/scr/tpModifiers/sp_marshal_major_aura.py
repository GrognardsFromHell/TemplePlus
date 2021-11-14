from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import aura_utils

print "Registering Marshal Major Auras"

def onEnterMarshalMajorAura(attachee, args, evt_obj):
    print "onEnterMarshalMajorAura Hook"
    auraEventId = args.get_arg(2)
    if auraEventId != evt_obj.evt_id:
        return 0
    auraSpellId = args.get_arg(0)
    auraSpellPacket = tpdp.SpellPacket(auraSpellId)
    auraTarget = evt_obj.target
    if auraTarget.is_friendly(attachee) and aura_utils.verifyTarget(auraTarget):
        if auraSpellPacket.add_target(auraTarget, 0):
            auraEnum = args.get_arg(1)
            auraName = aura_utils.getAuraName(auraEnum)
            auraTarget.condition_add_with_args("Marshal Major Aura {}".format(auraName), auraSpellId, auraEnum, auraEventId, 0, 0)
    return 0

def onConditionRemoveActions(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    spellPacket.remove_target(attachee)
    targetList = aura_utils.getTargetsInAura(spellPacket)
    for target in targetList:
        if target == OBJ_HANDLE_NULL:
            break
        target.d20_send_signal(S_Spell_End, spellId, 0)
        spellPacket.remove_target(target)
    return 0

marshalMajorAuraSpell = PythonModifier("sp-Marshal Major Aura", 5) #spell_id, activeAura, auraEventId, empty, empty
marshalMajorAuraSpell.AddHook(ET_OnObjectEvent, EK_OnEnterAoE, onEnterMarshalMajorAura, ())
marshalMajorAuraSpell.AddHook(ET_OnConditionRemove, EK_NONE, onConditionRemoveActions, ())
marshalMajorAuraSpell.AddSpellTeleportPrepareStandard()
marshalMajorAuraSpell.AddSpellTeleportReconnectStandard()
marshalMajorAuraSpell.AddAoESpellEndStandardHook()

### All Major Aura descriptions can be found at: http://archive.wizards.com/default.asp?x=dnd/ex/20030906b ###
### All Auras have 5 args: auraSpellId, auraEnum, auraEventId, empty, empty (may get reduced to 4 in final version)

### Marshal Major Aura Hardy Soldiers ###
def auraBonusHardySoldiers(attachee, args, evt_obj):
    auraSpellId = args.get_arg(0)
    auraSpellPacket = tpdp.SpellPacket(auraSpellId)
    auraBonus = aura_utils.getMajorAuraBonus(auraSpellPacket.caster_level)
    damageMesId = 126
    evt_obj.damage_packet.add_physical_damage_res(auraBonus, D20DAP_UNSPECIFIED, damageMesId)
    return 0

majorAuraHardySoldiers = aura_utils.AuraModifier("Marshal Major Aura Hardy Soldiers")
majorAuraHardySoldiers.AddHook(ET_OnTakingDamage2, EK_NONE, auraBonusHardySoldiers, ())
majorAuraHardySoldiers.marshalAuraAddPreActions()


### Marshal Major Aura Motivate Ardor ###
def auraBonusMotivateArdor(attachee, args, evt_obj):
    auraSpellId = args.get_arg(0)
    auraSpellPacket = tpdp.SpellPacket(auraSpellId)
    auraEnum = args.get_arg(1)
    auraName = aura_utils.getAuraName(auraEnum)
    auraTag = aura_utils.getAuraTag(auraEnum)
    auraBonus = aura_utils.getMajorAuraBonus(auraSpellPacket.caster_level)
    auraBonusType = 21 #ID 21 = Circumstance Bonus
    evt_obj.damage_packet.bonus_list.add(auraBonus, auraBonusType, "~Circumstance~[TAG_MODIFIER_CIRCUMSTANCE] : ~{}~[{}]".format(auraName, auraTag))
    return 0

majorAuraMotivateArdor = aura_utils.AuraModifier("Marshal Major Aura Motivate Ardor")
majorAuraMotivateArdor.AddHook(ET_OnDealingDamage2, EK_NONE, auraBonusMotivateArdor, ())
majorAuraMotivateArdor.marshalAuraAddPreActions()


### Marshal Major Aura Motivate Attack ###
def auraBonusMotivateAttack(attachee, args, evt_obj):
    if not evt_obj.attack_packet.get_flags() & D20CAF_RANGED:
        auraSpellId = args.get_arg(0)
        auraSpellPacket = tpdp.SpellPacket(auraSpellId)
        auraEnum = args.get_arg(1)
        auraName = aura_utils.getAuraName(auraEnum)
        auraTag = aura_utils.getAuraTag(auraEnum)
        auraBonus = aura_utils.getMajorAuraBonus(auraSpellPacket.caster_level)
        auraBonusType = 21 #ID 21 = Circumstance Bonus
        evt_obj.bonus_list.add(auraBonus, auraBonusType, "~Circumstance~[TAG_MODIFIER_CIRCUMSTANCE] : ~{}~[{}]".format(auraName, auraTag))
    return 0

majorAuraMotivateAttack = aura_utils.AuraModifier("Marshal Major Aura Motivate Attack")
majorAuraMotivateAttack.AddHook(ET_OnToHitBonus2, EK_NONE, auraBonusMotivateAttack, ())
majorAuraMotivateAttack.marshalAuraAddPreActions()

### Marshal Major Aura Motivate Care ###
majorAuraMotivateCare = aura_utils.AuraModifier("Marshal Major Aura Motivate Care")
majorAuraMotivateCare.AddHook(ET_OnGetAC, EK_NONE, aura_utils.auraAddBonusList, (aura_type_major,))
majorAuraMotivateCare.marshalAuraAddPreActions()

### Marshal Major Aura Motivate Urgency ###
def auraBonusMotivateUrgency(attachee, args, evt_obj):
    auraSpellId = args.get_arg(0)
    auraSpellPacket = tpdp.SpellPacket(auraSpellId)
    auraEnum = args.get_arg(1)
    auraName = aura_utils.getAuraName(auraEnum)
    auraTag = aura_utils.getAuraTag(auraEnum)
    auraBonus = aura_utils.getMajorAuraBonus(auraSpellPacket.caster_level)
    auraBonusType = 21 #ID 21 = Circumstance Bonus
    auraBonus *= 5
    evt_obj.bonus_list.add(auraBonus, auraBonusType, "~Circumstance~[TAG_MODIFIER_CIRCUMSTANCE] : ~{}~[{}]".format(auraName, auraTag))
    return 0

majorAuraMotivateUrgency = aura_utils.AuraModifier("Major Aura Motivate Urgency")
majorAuraMotivateUrgency.AddHook(ET_OnGetMoveSpeedBase, EK_NONE, auraBonusMotivateUrgency, ())
majorAuraMotivateUrgency.marshalAuraAddPreActions()


### Marshal Major Aura Resilient Troops" ###
majorAuraResilientTroops = aura_utils.AuraModifier("Marshal Major Aura Resilient Troops")
majorAuraResilientTroops.AddHook(ET_OnSaveThrowLevel, EK_NONE, aura_utils.auraAddBonusList, (aura_type_major,))
majorAuraResilientTroops.marshalAuraAddPreActions()

### Marshal Major Aura Steady Hand ###
def auraBonusSteadyHand(attachee, args, evt_obj):
    if evt_obj.attack_packet.get_flags() & D20CAF_RANGED:
        auraSpellId = args.get_arg(0)
        auraSpellPacket = tpdp.SpellPacket(auraSpellId)
        auraEnum = args.get_arg(1)
        auraName = aura_utils.getAuraName(auraEnum)
        auraTag = aura_utils.getAuraTag(auraEnum)
        auraBonus = aura_utils.getMajorAuraBonus(auraSpellPacket.caster_level)
        auraBonusType = 21 #ID 21 = Circumstance Bonus
        evt_obj.bonus_list.add(auraBonus, auraBonusType, "~Circumstance~[TAG_MODIFIER_CIRCUMSTANCE] : ~{}~[{}]".format(auraName, auraTag))
    return 0

majorAuraSteadyHand = aura_utils.AuraModifier("Marshal Major Aura Steady Hand")
majorAuraSteadyHand.AddHook(ET_OnToHitBonus2, EK_NONE, auraBonusSteadyHand, ())
majorAuraSteadyHand.marshalAuraAddPreActions()
