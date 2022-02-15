from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Distract"

def distractSpellTurnBasedStatusInit(attachee, args, evt_obj):
    if evt_obj.tb_status.hourglass_state > 2:
        attachee.float_text_line("Distracted", tf_red)
        evt_obj.tb_status.hourglass_state = 2 # Limited to a Standard or Move Action only
    return 0

distractSpell = SpellPythonModifier("sp-Distract") # spell_id, duration, empty
#Distract gives -4 penalty to Concentration, Listen, Search and Spot checks
distractSpell.AddHook(ET_OnTurnBasedStatusInit, EK_NONE, distractSpellTurnBasedStatusInit, ())
distractSpell.AddSkillBonus(-4, bonus_type_distract, skill_concentration, skill_listen, skill_search, skill_spot)
