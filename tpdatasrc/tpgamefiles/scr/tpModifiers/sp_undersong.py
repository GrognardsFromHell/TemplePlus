from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier, getSpellHelpTag

print "Registering sp-Undersong"

def undersongSpellReplaceConWithPerform(attachee, args, evt_obj):
    #Undersong replaces Concentration Checks with Perform Checks; to simplify it, add the diff as a generic stacking bonus
    bonusValue = attachee.skill_level_get(skill_perform) - attachee.skill_level_get(skill_concentration)
    if bonusValue > 0: #Undersong is a may condition
        bonusType = bonus_type_undersong
        bonusHelpTag = game.get_mesline("mes\\bonus_description.mes", bonusType)
        spellId = args.get_arg(0)
        spellHelpTag = getSpellHelpTag(spellId)
        evt_obj.bonus_list.add(bonusValue, bonusType, "{} : {}".format(bonusHelpTag, spellHelpTag))
    return 0

undersongSpell = SpellPythonModifier("sp-Undersong") # spellId, duration, empty
undersongSpell.AddHook(ET_OnGetSkillLevel, EK_SKILL_CONCENTRATION, undersongSpellReplaceConWithPerform,())
