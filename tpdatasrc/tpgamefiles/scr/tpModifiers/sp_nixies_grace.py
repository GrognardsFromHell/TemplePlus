from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Nixies Grace"

### Swim Speed and breath underwater is not applicable in ToEE ###

### Is low light vision of revelance? ###

def nixiesGraceSpellDexterityBonus(attachee, args, evt_obj):
    bonusValue = 6 #Nixies Grace adds a +6 Enhancement Bonus to Dexterity
    bonusType = 12 #ID 12 = Enhancement
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Enhancement~[TAG_ENHANCEMENT_BONUS] : ~Nixies Grace~[TAG_SPELLS_NIXIES_GRACE]")
    return 0

def nixiesGraceSpellWisdomBonus(attachee, args, evt_obj):
    bonusValue = 2 #Nixies Grace adds a +2 Enhancement Bonus to Wisdom
    bonusType = 12 #ID 12 = Enhancement
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Enhancement~[TAG_ENHANCEMENT_BONUS] : ~Nixies Grace~[TAG_SPELLS_NIXIES_GRACE]")
    return 0

def nixiesGraceSpellCharismaBonus(attachee, args, evt_obj):
    bonusValue = 8 #Nixies Grace adds a +8 Enhancement Bonus to Charisma
    bonusType = 12 #ID 12 = Enhancement
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Enhancement~[TAG_ENHANCEMENT_BONUS] : ~Nixies Grace~[TAG_SPELLS_NIXIES_GRACE]")
    return 0

def nixiesGraceSpellColdIronDr(attachee, args, evt_obj):
    drAmount = 5
    drBreakType = D20DAP_COLD #COLD = Cold Iron
    damageMesId = 126 #ID126 in damage.mes is DR
    evt_obj.damage_packet.add_physical_damage_res(drAmount, drBreakType, damageMesId)
    return 0

nixiesGraceSpell = PythonModifier("sp-Nixies Grace", 3, False) # spell_id, duration, empty
nixiesGraceSpell.AddHook(ET_OnTakingDamage, EK_NONE, nixiesGraceSpellColdIronDr,())
nixiesGraceSpell.AddHook(ET_OnAbilityScoreLevel, EK_STAT_CHARISMA, nixiesGraceSpellCharismaBonus,())
nixiesGraceSpell.AddHook(ET_OnAbilityScoreLevel, EK_STAT_DEXTERITY, nixiesGraceSpellDexterityBonus,())
nixiesGraceSpell.AddHook(ET_OnAbilityScoreLevel, EK_STAT_WISDOM, nixiesGraceSpellWisdomBonus,())
nixiesGraceSpell.AddHook(ET_OnConditionAddPre, EK_NONE, spell_utils.replaceCondition, ()) #damage reduction does stack; so I need replaceCondition
nixiesGraceSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
nixiesGraceSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
nixiesGraceSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
nixiesGraceSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
nixiesGraceSpell.AddSpellDispelCheckStandard()
nixiesGraceSpell.AddSpellTeleportPrepareStandard()
nixiesGraceSpell.AddSpellTeleportReconnectStandard()
nixiesGraceSpell.AddSpellCountdownStandardHook()
