# Natural weapon blend: conditions that grant natural attacks usable alongside manufactured weapons.
# Engine side: ET_OnGetExtraNaturalAttacks (TemplePlus action_sequence.cpp, namespace naturalBlend).
#  - armed: extra natural attacks come after the iterative weapon attacks as secondary attacks
#    (-5 to hit, -2 with "Has Multiattack"; half Str damage)
#  - unarmed (and no proto natural attacks): they replace unarmed strikes (primary, full Str)
#  - NAF_NEEDS_FREE_HAND attacks need a free hand (a buckler leaves it free, any other shield blocks it)
#  - NAF_MOUTH attacks are dropped while the "Mouth Occupied" query is true
from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering natural_weapon_blend"

def _yes(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

### Dragon Disciple-style claws + bite (prototype / test condition)
# arg0: unused, arg1: unused
def DDNaturalAttacks(attachee, args, evt_obj):
    size = attachee.get_size
    if size <= STAT_SIZE_SMALL:
        claw, bite = dice_new("1d3"), dice_new("1d4")
    elif size >= STAT_SIZE_LARGE:
        claw, bite = dice_new("1d6"), dice_new("1d8")
    else:
        claw, bite = dice_new("1d4"), dice_new("1d6")
    evt_obj.add_attack(NATURAL_ATTACK_CLAW, claw, D20DT_SLASHING, NAF_PRIMARY | NAF_NEEDS_FREE_HAND)
    evt_obj.add_attack(NATURAL_ATTACK_CLAW, claw, D20DT_SLASHING, NAF_PRIMARY | NAF_NEEDS_FREE_HAND)
    evt_obj.add_attack(NATURAL_ATTACK_BITE, bite, D20DT_PIERCING, NAF_PRIMARY | NAF_MOUTH)
    return 0

ddNat = PythonModifier("Natural Weapons Test DD", 2)
ddNat.AddHook(ET_OnGetExtraNaturalAttacks, EK_NONE, DDNaturalAttacks, ())

### Monster opt-in: use the proto natural attacks together with a wielded weapon
# arg0: multiattack mode: 0 = auto (non-humanoids have Multiattack), 1 = always, 2 = never
def ProtoNaturalAttacks(attachee, args, evt_obj):
    evt_obj.add_proto_attacks()
    return 0

def ProtoMultiattack(attachee, args, evt_obj):
    mode = args.get_arg(0)
    if mode == 1 or (mode == 0 and not attachee.is_category_type(mc_type_humanoid)):
        evt_obj.return_val = 1
    return 0

protoNat = PythonModifier("Natural Attacks While Armed", 2)
protoNat.AddHook(ET_OnGetExtraNaturalAttacks, EK_NONE, ProtoNaturalAttacks, ())
protoNat.AddHook(ET_OnD20PythonQuery, "Has Multiattack", ProtoMultiattack, ())

### Multiattack (read only by the blended path)
multiattack = PythonModifier("Multiattack", 2)
multiattack.AddHook(ET_OnD20PythonQuery, "Has Multiattack", _yes, ())
