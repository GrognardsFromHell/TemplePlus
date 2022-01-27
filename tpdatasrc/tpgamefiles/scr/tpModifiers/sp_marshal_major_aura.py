from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import aura_utils

print "Registering Marshal Major Auras"

marshalMajorAuraSpell = aura_utils.AuraAoeHandlingModifier("sp-Marshal Major Aura")

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
    auraBonusType = bonus_type_circumstance #Stacking!
    evt_obj.damage_packet.bonus_list.add(auraBonus, auraBonusType, "Marshal Major Aura: ~{}~[{}]".format(auraName, auraTag))
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
        auraBonusType = bonus_type_circumstance #Stacking!
        evt_obj.bonus_list.add(auraBonus, auraBonusType, "Marshal Major Aura: ~{}~[{}]".format(auraName, auraTag))
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
    auraBonusType = bonus_type_circumstance #Stacking!
    auraBonus *= 5
    evt_obj.bonus_list.add(auraBonus, auraBonusType, "Marshal Major Aura: ~{}~[{}]".format(auraName, auraTag))
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
        auraBonusType = bonus_type_circumstance #Stacking!
        evt_obj.bonus_list.add(auraBonus, auraBonusType, "Marshal Major Aura: ~{}~[{}]".format(auraName, auraTag))
    return 0

majorAuraSteadyHand = aura_utils.AuraModifier("Marshal Major Aura Steady Hand")
majorAuraSteadyHand.AddHook(ET_OnToHitBonus2, EK_NONE, auraBonusSteadyHand, ())
majorAuraSteadyHand.marshalAuraAddPreActions()
