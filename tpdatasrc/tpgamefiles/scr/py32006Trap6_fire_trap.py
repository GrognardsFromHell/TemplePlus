from toee import *

def san_trap( trap, triggerer ):
	game.particles( trap.partsys, trap.obj )
	for obj in game.obj_list_vicinity( triggerer.location, OLC_CRITTERS ):
		if (obj.distance_to(trap.obj) <= 5):
			if (obj.has_los(trap.obj)):
				for dmg in trap.damage:
					if (dmg.type == D20DT_POISON):
						if (obj.saving_throw( 15, D20_Save_Fortitude, D20STD_F_POISON, trap.obj ) == 0):
							obj.condition_add_with_args("Poisoned",dmg.damage.bonus,0)
					else:
						obj.reflex_save_and_damage( trap.obj, 16, D20_Save_Reduction_Half, D20STD_F_SPELL_DESCRIPTOR_FIRE, dmg.damage, dmg.type, D20DAP_NORMAL )
	
	print "Trap script ID changed to 0"
	trap.obj.scripts[39] = 0 # fixes re-arming when doing disable device
	return SKIP_DEFAULT
