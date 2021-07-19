from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Sirines Grace"

### Swim Speed and breath underwater is not applicable in ToEE nor the underwater combat changes ###

def sirinesGraceSpellAbilityBonus(attachee, args, evt_obj):
    bonusValue = 4 #Sirines Grace adds a +4 Enhancement Bonus to Charisma and Dexterity
    bonusType = 12 #ID 12 = Enhancement
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Enhancement~[TAG_ENHANCEMENT_BONUS] : ~Sirines Grace~[TAG_SPELLS_SIRINES_GRACE]")
    return 0

def sirinesGraceSpellPerformBonus(attachee, args, evt_obj):
    bonusValue = 8 #Sirines Grace adds a +8 untyped Bonus to Perform
    bonusType = 155 #New ID for Sirines Grace
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Sirines Grace~[TAG_SPELLS_SIRINES_GRACE] Bonus")
    return 0

def sirinesGraceSpellAcBonus(attachee, args, evt_obj):
    bonusValue = max(0, int(((attachee.stat_level_get(stat_charisma)-10)/2))) #Sirines Grace adds the Charisma Modifier Value as Deflection Bonus to AC
    bonusType = 11 #ID 11 = Deflection
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Deflection~[TAG_DEFLECTION_BONUS] : ~Sirines Grace~[TAG_SPELLS_SIRINES_GRACE]")
    return 0

sirinesGraceSpell = PythonModifier("sp-Sirines Grace", 3, False) # spell_id, duration, empty
sirinesGraceSpell.AddHook(ET_OnGetAC, EK_NONE, sirinesGraceSpellAcBonus,())
sirinesGraceSpell.AddHook(ET_OnGetSkillLevel, EK_SKILL_PERFORM, sirinesGraceSpellPerformBonus,())
sirinesGraceSpell.AddHook(ET_OnAbilityScoreLevel, EK_STAT_CHARISMA, sirinesGraceSpellAbilityBonus,())
sirinesGraceSpell.AddHook(ET_OnAbilityScoreLevel, EK_STAT_DEXTERITY, sirinesGraceSpellAbilityBonus,())
sirinesGraceSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
sirinesGraceSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
sirinesGraceSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
sirinesGraceSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
sirinesGraceSpell.AddSpellDispelCheckStandard()
sirinesGraceSpell.AddSpellTeleportPrepareStandard()
sirinesGraceSpell.AddSpellTeleportReconnectStandard()
sirinesGraceSpell.AddSpellCountdownStandardHook()
