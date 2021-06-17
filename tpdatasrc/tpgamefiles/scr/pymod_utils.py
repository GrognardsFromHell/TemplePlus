from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *

def preventDupSameArg(attachee, args, evt_obj):
    cond_arg0 = args.get_arg(0)
    evt_obj_arg = evt_obj.arg1
    my_name = args.get_cond_name()
    
    if evt_obj.is_modifier(my_name) and cond_arg0 == evt_obj_arg:
        evt_obj.return_val = 0
    
    return 0
