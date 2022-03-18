from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import aura_utils

print "Registering Marshal Minor Auras"

marshalMinorAuraSpell = aura_utils.AuraAoeHandlingModifier("sp-Marshal Minor Aura")

### All Minor Aura descriptions can be found at: http://archive.wizards.com/default.asp?x=dnd/ex/20030906b ###
### All Auras have 5 args: auraSpellId, auraEnum, auraEventId, empty, empty (may get reduced to 4 in final version)

### Marshal Minor Aura Accurate Strike ###
marshalMinorAuraAccurateStrike = aura_utils.AuraModifier("Marshal Minor Aura Accurate Strike")
marshalMinorAuraAccurateStrike.AddHook(ET_OnConfirmCriticalBonus, EK_NONE, aura_utils.auraAddBonusList, (aura_type_minor,))
marshalMinorAuraAccurateStrike.marshalAuraAddPreActions()

### Marshal Minor Aura Art of War ###
marshalMinorAuraArtOfWar = aura_utils.AuraModifier("Marshal Minor Aura Art of War")
marshalMinorAuraArtOfWar.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE, aura_utils.auraAddBonusList, (aura_type_minor,)) #are there AbilityCheckModifier that are not disarm, trip, bull rush, or sunder? If yes redo!
marshalMinorAuraArtOfWar.marshalAuraAddPreActions()

### Marshal Minor Aura Demand Fortitude ###
marshalMinorAuraDemandFortitude = aura_utils.AuraModifier("Marshal Minor Aura Demand Fortitude")
marshalMinorAuraDemandFortitude.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, aura_utils.auraAddBonusList, (aura_type_minor,))
marshalMinorAuraDemandFortitude.marshalAuraAddPreActions()

### Marshal Minor Aura Determined Caster ###
marshalMinorAuraDeterminedCaster= aura_utils.AuraModifier("Marshal Minor Aura Determined Caster")
marshalMinorAuraDeterminedCaster.AddHook(ET_OnSpellResistanceCheckBonus, EK_NONE, aura_utils.auraAddBonusList, (aura_type_minor,))
marshalMinorAuraDeterminedCaster.marshalAuraAddPreActions()

### Marshal Minor Aura Force of Will ###
marshalMinorAuraForceOfWill = aura_utils.AuraModifier("Marshal Minor Aura Force of Will")
marshalMinorAuraForceOfWill.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, aura_utils.auraAddBonusList, (aura_type_minor,))
marshalMinorAuraForceOfWill.marshalAuraAddPreActions()

### Marshal Minor Aura Master of Opportunity ###
def auraBonusMasterOfOpportunity(attachee, args, evt_obj):
    if evt_obj.attack_packet.get_flags() & D20CAF_ATTACK_OF_OPPORTUNITY:
        auraSpellId = args.get_arg(0)
        auraSpellPacket = tpdp.SpellPacket(auraSpellId)
        auraEnum = args.get_arg(1)
        auraName = aura_utils.getAuraName(auraEnum)
        auraTag = aura_utils.getAuraTag(auraEnum)
        auraBonus = aura_utils.getMinorAuraBonus(auraSpellPacket.caster)
        auraBonusType = 21 #ID 21 = Circumstance Bonus
        evt_obj.bonus_list.add(auraBonus, auraBonusType,"~Circumstance~[TAG_MODIFIER_CIRCUMSTANCE] : ~{}~[{}]".format(auraName, auraTag))
    return 0

marshalMinorAuraMasterOfOpportunity = aura_utils.AuraModifier("Marshal Minor Aura Master of Opportunity")
marshalMinorAuraMasterOfOpportunity.AddHook(ET_OnGetAC, EK_NONE, auraBonusMasterOfOpportunity, ())
marshalMinorAuraMasterOfOpportunity.marshalAuraAddPreActions()

### Marshal Minor Aura Master of Tactics ###
def auraBonusMasterOfTactics(attachee, args, evt_obj):
    print "auraBonusMasterOfTactics Hook"
    if evt_obj.attack_packet.get_flags() & D20CAF_FLANKED:
        auraSpellId = args.get_arg(0)
        auraSpellPacket = tpdp.SpellPacket(auraSpellId)
        auraEnum = args.get_arg(1)
        auraName = aura_utils.getAuraName(auraEnum)
        auraTag = aura_utils.getAuraTag(auraEnum)
        auraBonus = aura_utils.getMinorAuraBonus(auraSpellPacket.caster)
        auraBonusType = 21 #ID 21 = Circumstance Bonus
        evt_obj.damage_packet.bonus_list.add(auraBonus, auraBonusType,"~Circumstance~[TAG_MODIFIER_CIRCUMSTANCE] : ~{}~[{}]".format(auraName, auraTag))
    return 0

marshalMinorAuraMasterOfTactics = aura_utils.AuraModifier("Marshal Minor Aura Master of Tactics")
marshalMinorAuraMasterOfTactics.AddHook(ET_OnDealingDamage2, EK_NONE, auraBonusMasterOfTactics, ())
marshalMinorAuraMasterOfTactics.marshalAuraAddPreActions()

### Marshal Minor Aura Motivate Charisma ###
marshalMinorAuraMotivateCharisma = aura_utils.AuraModifier("Marshal Minor Aura Motivate Charisma")
marshalMinorAuraMotivateCharisma.AddHook(ET_OnGetAbilityCheckModifier, EK_STAT_CHARISMA, aura_utils.auraAddBonusList, (aura_type_minor,))
marshalMinorAuraMotivateCharisma.addSkillBonus(aura_type_minor, skill_bluff, skill_diplomacy, skill_disguise, skill_gather_information, skill_handle_animal,
skill_intimidate, skill_perform, skill_use_magic_device)
marshalMinorAuraMotivateCharisma.marshalAuraAddPreActions()

### Marshal Minor Aura Motivate Constitution ###
marshalMinorAuraMotivateConstitution = aura_utils.AuraModifier("Marshal Minor Aura Motivate Constitution")
marshalMinorAuraMotivateConstitution.AddHook(ET_OnGetAbilityCheckModifier, EK_STAT_CONSTITUTION, aura_utils.auraAddBonusList, (aura_type_minor,))
marshalMinorAuraMotivateConstitution.addSkillBonus(aura_type_minor, skill_concentration)
marshalMinorAuraMotivateConstitution.marshalAuraAddPreActions()

### Marshal Minor Aura Motivate Dexterity ###
marshalMinorAuraMotivateDexterity = aura_utils.AuraModifier("Marshal Minor Aura Motivate Dexterity")
marshalMinorAuraMotivateDexterity.AddHook(ET_OnGetAbilityCheckModifier, EK_STAT_DEXTERITY, aura_utils.auraAddBonusList, (aura_type_minor,))
marshalMinorAuraMotivateDexterity.addSkillBonus(aura_type_minor, skill_balance, skill_balance, skill_escape_artist, skill_hide, skill_move_silently,
skill_open_lock, skill_ride, skill_pick_pocket, skill_tumble, skill_use_rope)
marshalMinorAuraMotivateDexterity.marshalAuraAddPreActions()

### Marshal Minor Aura Motivate Intelligence ###
marshalMinorAuraMotivateIntelligence = aura_utils.AuraModifier("Marshal Minor Aura Motivate Intelligence")
marshalMinorAuraMotivateIntelligence.AddHook(ET_OnGetAbilityCheckModifier, EK_STAT_INTELLIGENCE, aura_utils.auraAddBonusList, (aura_type_minor,))
marshalMinorAuraMotivateIntelligence.addSkillBonus(aura_type_minor, skill_appraise, skill_craft, skill_decipher_script, skill_disable_device, skill_forgery,
skill_knowledge_arcana, skill_knowledge_religion, skill_knowledge_nature, skill_search, skill_spellcraft)
#marshalMinorAuraMotivateIntelligence.AddHook(ET_OnGetSkillLevel, EK_SKILL_KNOWLEDGE_ALL, auraAddBonusList, ())
marshalMinorAuraMotivateIntelligence.marshalAuraAddPreActions()

### Marshal Minor Aura Motivate Strength ###
marshalMinorAuraMotivateStrength = aura_utils.AuraModifier("Marshal Minor Aura Motivate Strength")
marshalMinorAuraMotivateStrength.AddHook(ET_OnGetAbilityCheckModifier, EK_STAT_STRENGTH, aura_utils.auraAddBonusList, (aura_type_minor,))
marshalMinorAuraMotivateStrength.addSkillBonus(aura_type_minor, skill_climb, skill_jump, skill_swim)
marshalMinorAuraMotivateStrength.marshalAuraAddPreActions()

### Marshal Minor Aura Motivate Wisdom ###
marshalMinorAuraMotivateWisdom = aura_utils.AuraModifier("Marshal Minor Aura Motivate Wisdom")
marshalMinorAuraMotivateWisdom.AddHook(ET_OnGetAbilityCheckModifier, EK_STAT_WISDOM, aura_utils.auraAddBonusList, (aura_type_minor,))
marshalMinorAuraMotivateWisdom.addSkillBonus(aura_type_minor, skill_heal, skill_listen, skill_sense_motive, skill_spot, skill_wilderness_lore)
marshalMinorAuraMotivateWisdom.marshalAuraAddPreActions()

### Marshal Minor Aura Over the Top ###
def auraBonusOverTheTop(attachee, args, evt_obj):
    if evt_obj.attack_packet.get_flags() & D20CAF_CHARGE:
        auraSpellId = args.get_arg(0)
        auraSpellPacket = tpdp.SpellPacket(auraSpellId)
        auraEnum = args.get_arg(1)
        auraName = aura_utils.getAuraName(auraEnum)
        auraTag = aura_utils.getAuraTag(auraEnum)
        auraBonus = aura_utils.getMinorAuraBonus(auraSpellPacket.caster)
        auraBonusType = 21 #ID 21 = Circumstance Bonus
        evt_obj.damage_packet.bonus_list.add(auraBonus, auraBonusType,"~Circumstance~[TAG_MODIFIER_CIRCUMSTANCE] : ~{}~[{}]".format(auraName, auraTag))
    return 0

marshalMinorAuraOverTheTop = aura_utils.AuraModifier("Marshal Minor Aura Over the Top")
marshalMinorAuraOverTheTop.AddHook(ET_OnDealingDamage2, EK_NONE, auraBonusOverTheTop, ())
marshalMinorAuraOverTheTop.marshalAuraAddPreActions()

### Marshal Minor Aura Watchful Eye ###
marshalMinorAuraWatchfulEye = aura_utils.AuraModifier("Marshal Minor Aura Watchful Eye")
marshalMinorAuraWatchfulEye.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, aura_utils.auraAddBonusList, (aura_type_minor,))
marshalMinorAuraWatchfulEye.marshalAuraAddPreActions()
