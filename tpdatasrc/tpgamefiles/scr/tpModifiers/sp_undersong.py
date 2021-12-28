from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Undersong"

def undersongSpellReplaceConWithPerform(attachee, args, evt_obj):
    #Undersong replaces Concentration Checks with Perform Checks; to simplify it, add the diff as a generic stacking bonus
    bonusValue = attachee.skill_level_get(skill_perform) - attachee.skill_level_get(skill_concentration)
    bonusType = 157 #New ID for Undersong
    if bonusValue > 0: #Undersong is a may condition
        evt_obj.bonus_list.add(bonusValue, bonusType, "~Undersong~[TAG_SPELLS_UNDERSONG] Bonus")
    return 0

undersongSpell = SpellPythonModifier("sp-Undersong") # spellId, duration, empty
undersongSpell.AddHook(ET_OnGetSkillLevel, EK_SKILL_CONCENTRATION, undersongSpellReplaceConWithPerform,())
