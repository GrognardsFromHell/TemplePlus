from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Tripping Bite"

def OnDamage2(attachee, args, evt_obj):
	#Check that it is an attack and not a spell to avoid spells like flame strike making trip attempts in wildshape!
	action_type = evt_obj.attack_packet.action_type
	if action_type != D20A_FULL_ATTACK and action_type != D20A_STANDARD_ATTACK:
		return
	
	target = evt_obj.attack_packet.target
	if (target != OBJ_HANDLE_NULL):
		if (target.d20_query(Q_Prone) == 0 and attachee.trip_check(target)):
			target.fall_down()
			target.condition_add("Prone")
			target.float_mesfile_line( 'mes\\combat.mes', 104, 1 ) # Tripped!

tripBite = PythonModifier("Tripping Bite", 0)
tripBite.AddHook(ET_OnDealingDamage2, EK_NONE, OnDamage2, ())

