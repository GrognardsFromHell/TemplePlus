from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering sp-Wall of Fire"


def WallOfFireOnAdd(attachee, args, evt_obj):
    spell_id = args.get_arg(0)
    spell_packet = tpdp.SpellPacket(spell_id)

    wall_length_ft = 5.0
    wall_angle_rad = 1.5
    evt_id = attachee.object_event_append_wall(OLC_CRITTERS, wall_length_ft, 10)
    args.set_arg(2, evt_id) # store the event ID
    spell_packet.set_spell_object(attachee, args.get_arg(3)) # store the spell obj and the particle sys
    spell_packet.update_registry()
    return 0

wallOfFire = PythonModifier("sp-Wall of Fire", 8)
wallOfFire.AddHook(ET_OnConditionAdd, EK_NONE, WallOfFireOnAdd, ())



