from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier, skillCheck

print "Registering sp-Dirge of Discord"

def dirgeOfDiscordSpellMovementPenalty(attachee, args, evt_obj):
    #Dirge of Discord recudes speed by 50% to a minimum of 5
    moveSpeedBase = attachee.stat_level_get(stat_movement_speed)
    bonusValue = -(moveSpeedBase/2)
    evt_obj.bonus_list.add(bonusValue, bonus_type_untyped, "~Dirge of Discord~[TAG_SPELLS_DIRGE_OF_DISCORD] Penalty")
    newSpeed = evt_obj.bonus_list.get_sum()
    if newSpeed < 5:
        speedToAdd = 5 - newSpeed
        evt_obj.bonus_list.add(speedToAdd, bonus_type_untyped, "~Dirge of Discord~[TAG_SPELLS_DIRGE_OF_DISCORD] reduces to a minimum of 5 speed")
    return 0

def dirgeOfDiscordSpellConcentrationCheck(attachee, args, evt_obj):
    spellPacket = evt_obj.get_spell_packet()
    spellLevel = spellPacket.spell_known_slot_level #not tested with metamagic hightend spells
    spellDc = args.get_arg(2)
    skillCheckDc = spellDc + spellLevel
    if not skillCheck(attachee, skill_concentration, skillCheckDc):
        attachee.float_text_line("Spell failed", tf_red)
        #game.particles('Fizzle', attachee)
        evt_obj.return_val = 100
    return 0

spellPenalty = -4

dirgeOfDiscordSpell = SpellPythonModifier("sp-Dirge of Discord", 4) # spell_id, duration, spellDc, empty
dirgeOfDiscordSpell.AddHook(ET_OnGetMoveSpeedBase, EK_NONE, dirgeOfDiscordSpellMovementPenalty,())
dirgeOfDiscordSpell.AddHook(ET_OnD20Query, EK_Q_SpellInterrupted, dirgeOfDiscordSpellConcentrationCheck,())
dirgeOfDiscordSpell.AddToHitBonus(spellPenalty, bonus_type_untyped)
dirgeOfDiscordSpell.AddAbilityBonus(spellPenalty, bonus_type_untyped, stat_dexterity)
dirgeOfDiscordSpell.AddSpellConcentration()
dirgeOfDiscordSpell.AddSpellNoDuplicate()
