from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Serene Visage"

def sereneVisageSpellBonusToBluff(attachee, args, evt_obj):
    bonusValue = args.get_arg(2)
    bonusType = 18 #ID 18 = Insight
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Serene Visage~[TAG_SPELLS_SERENE_VISAGE] ~Insight~[TAG_MODIFIER_INSIGHT] Bonus")
    return 0

sereneVisageSpell = PythonModifier("sp-Serene Visage", 4, False) # spell_id, duration, bonusValue, empty
sereneVisageSpell.AddHook(ET_OnGetSkillLevel, EK_SKILL_BLUFF, sereneVisageSpellBonusToBluff,())
sereneVisageSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
sereneVisageSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
sereneVisageSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
sereneVisageSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
sereneVisageSpell.AddSpellDispelCheckStandard()
sereneVisageSpell.AddSpellTeleportPrepareStandard()
sereneVisageSpell.AddSpellTeleportReconnectStandard()
sereneVisageSpell.AddSpellCountdownStandardHook()
