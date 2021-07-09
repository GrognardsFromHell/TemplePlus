from templeplus.pymod import PythonModifier
from toee import *
import tpdp

### Generic Monster DR Condition

def grantMonsterDr(attachee, args, evt_obj):
    drAmount = args.get_arg(0)
    drBreakType = args.get_arg(1)
    damageMesId = 126 #ID126 in damage.mes = ~Damage Reduction~[TAG_SPECIAL_ABILITIES_DAMAGE_REDUCTION]
    evt_obj.damage_packet.add_physical_damage_res(drAmount, drBreakType, damageMesId)
    return 0

def breakCritterDr(attachee, args, evt_obj):
    drBreakType = args.get_arg(1)
    if not evt_obj.damage_packet.attack_power & drBreakType:
        evt_obj.damage_packet.attack_power |= drBreakType
    return 0

genericMonsterDR = PythonModifier("Generic Monster DR", 2) #arg1: DR Amount, arg2: DR Type
genericMonsterDR.AddHook(ET_OnTakingDamage2, EK_NONE, grantMonsterDr, ())
genericMonsterDR.AddHook(ET_OnDealingDamage, EK_NONE, breakCritterDr, ())
