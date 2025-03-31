from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Find the Gap"

def negateArmor(attachee, args, evt_obj):
    activatedFlag = args.get_arg(2)
    if not activatedFlag: #only the first attack each round ignores armors
        flags = evt_obj.attack_packet.get_flags()
        if not flags & D20CAF_TOUCH_ATTACK:
            description = "(~Find the Gap~[TAG_SPELLS_FIND_THE_GAP]"
            evt_obj.bonus_list.add_cap(9 , 0, 1, description)
            evt_obj.bonus_list.add_cap(10 , 0, 1, description)
            evt_obj.bonus_list.add_cap(12 , 0, 1, description)
            evt_obj.bonus_list.add_cap(28 , 0, 1, description)
            evt_obj.bonus_list.add_cap(29 , 0, 1, description)
            evt_obj.bonus_list.add_cap(33 , 0, 1, description)
            attachee.float_text_line("Find the Gap!")
        #Set Touch Attack Flag
        args.set_arg(2, 1)
    return 0

def resetFlag(attachee, args, evt_obj):
    args.set_arg(2, 0)
    return 0

findTheGapSpell = SpellPythonModifier("sp-Find the Gap", 4) # spell_id, duration, activatedFlag, empty
findTheGapSpell.AddHook(ET_OnGetAcModifierFromAttacker , EK_NONE, negateArmor, ())
findTheGapSpell.AddHook(ET_OnBeginRound, EK_NONE, resetFlag, ())
