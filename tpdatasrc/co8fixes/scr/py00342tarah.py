from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	return RUN_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_vars[993] == 2):
		attachee.object_flag_unset(OF_OFF)
	elif (game.global_vars[993] == 3):
		attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[949] = 1
	game.global_vars[993] = 5
	game.sound( 4112, 1 )
	if (game.global_flags[948] == 1 and game.global_flags[950] == 1 and game.global_flags[951] == 1 and game.global_flags[952] == 1 and game.global_flags[953] == 1 and game.global_flags[954] == 1):
		game.party[0].reputation_add(40)
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[949] = 0
	game.party[0].reputation_remove(40)
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (is_better_to_talk(attachee, obj)):
				attachee.cast_spell(spell_stoneskin, attachee)
				game.new_sid = 0
	return RUN_DEFAULT


def is_better_to_talk(speaker,listener):
	if (speaker.distance_to(listener) <= 55):
		return 1
	return 0


def switch_to_kenan( attachee, triggerer, line):
	npc = find_npc_near(attachee,8804)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
	return SKIP_DEFAULT


def switch_to_sharar( attachee, triggerer, line):
	npc = find_npc_near(attachee,8806)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
	return SKIP_DEFAULT


def switch_to_gadham( attachee, triggerer, line):
	npc = find_npc_near(attachee,8807)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
	return SKIP_DEFAULT


def switch_to_abaddon( attachee, triggerer, line):
	npc = find_npc_near(attachee,8808)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
	return SKIP_DEFAULT


def switch_to_gershom( attachee, triggerer, line):
	npc = find_npc_near(attachee,8810)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
	return SKIP_DEFAULT


def switch_to_daniel( attachee, triggerer, line):
	npc = find_npc_near(attachee,8720)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
		npc.turn_towards(attachee)
	return SKIP_DEFAULT


def switch_to_meleny( attachee, triggerer, line):
	npc = find_npc_near(attachee,8015)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
		npc.turn_towards(attachee)
	return SKIP_DEFAULT


def switch_to_riana( attachee, triggerer, line):
	npc = find_npc_near(attachee,8058)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
		npc.turn_towards(attachee)
	return SKIP_DEFAULT


def switch_to_fruella( attachee, triggerer, line):
	npc = find_npc_near(attachee,8067)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
		npc.turn_towards(attachee)
	return SKIP_DEFAULT


def switch_to_serena( attachee, triggerer, line):
	npc = find_npc_near(attachee,8056)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
		npc.turn_towards(attachee)
	return SKIP_DEFAULT


def switch_to_pishella( attachee, triggerer, line):
	npc = find_npc_near(attachee,8069)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
		npc.turn_towards(attachee)
	return SKIP_DEFAULT


def switch_to_kella( attachee, triggerer, line):
	npc = find_npc_near(attachee,8070)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
		npc.turn_towards(attachee)
	return SKIP_DEFAULT


def pick_to_grope( attachee, triggerer ):
	if anyone( triggerer.group_list(), "has_follower", 8015 ):
		triggerer.begin_dialog( attachee, 70 )
	elif anyone( triggerer.group_list(), "has_follower", 8058 ):
		triggerer.begin_dialog( attachee, 100 )
	elif anyone( triggerer.group_list(), "has_follower", 8067 ):
		triggerer.begin_dialog( attachee, 80 )
	elif anyone( triggerer.group_list(), "has_follower", 8056 ):
		triggerer.begin_dialog( attachee, 90 )
	elif anyone( triggerer.group_list(), "has_follower", 8069 ):
		triggerer.begin_dialog( attachee, 260 )
	elif anyone( triggerer.group_list(), "has_follower", 8070 ):
		triggerer.begin_dialog( attachee, 270 )
	else:
		triggerer.begin_dialog( attachee, 110 )
	return SKIP_DEFAULT


def create_skel( attachee, triggerer ):
	skel1 = game.obj_create(14602, location_from_axis( 498, 575 ))
	skel1.rotation = 2.5
	skel1.concealed_set(1)
	skel1.unconceal()
	game.particles( "Trap-Spores", skel1 )
	skel2 = game.obj_create(14092, location_from_axis( 500, 572 ))
	skel2.rotation = 2.5
	skel2.concealed_set(1)
	skel2.unconceal()
	game.particles( "Trap-Spores", skel2 )
	skel3 = game.obj_create(14092, location_from_axis( 502, 571 ))
	skel3.rotation = 2.5
	skel3.concealed_set(1)
	skel3.unconceal()
	game.particles( "Trap-Spores", skel3 )
	skel4 = game.obj_create(14092, location_from_axis( 495, 575 ))
	skel4.rotation = 3.0
	skel4.concealed_set(1)
	skel4.unconceal()
	game.particles( "Trap-Spores", skel4 )
	skel5 = game.obj_create(14092, location_from_axis( 492, 576 ))
	skel5.rotation = 3.0
	skel5.concealed_set(1)
	skel5.unconceal()
	game.particles( "Trap-Spores", skel5 )
	game.sound( 4015, 1 )
	return RUN_DEFAULT


def destroy_skel( attachee, triggerer ):
	skel = find_npc_near(attachee,14602)
	abby = find_npc_near(attachee,8808)
	abby.turn_towards(skel)
	game.particles( "cast-Necromancy-cast", abby )
	game.particles( "hit-UNHOLY-medium", skel )
	skel.object_flag_set(OF_OFF)
	zomb = find_npc_near(attachee,14092)
	game.particles( "hit-UNHOLY-medium", zomb )
	zomb.object_flag_set(OF_OFF)
	zomb = find_npc_near(attachee,14092)
	game.particles( "hit-UNHOLY-medium", zomb )
	zomb.object_flag_set(OF_OFF)
	zomb = find_npc_near(attachee,14092)
	game.particles( "hit-UNHOLY-medium", zomb )
	zomb.object_flag_set(OF_OFF)
	zomb = find_npc_near(attachee,14092)
	game.particles( "hit-UNHOLY-medium", zomb )
	zomb.object_flag_set(OF_OFF)
	game.sound( 4113, 1 )
	return RUN_DEFAULT


def pick_random_four( attachee, triggerer ):	##doesn't work reliably
	rr = game.random_range(3,4)
	pc = game.party[rr]
	pc.critter_kill_by_effect()
	game.particles( "sp-Slay Living", pc )
	game.particles( "ef-MinoCloud", pc )
	return


def pick_random_five( attachee, triggerer ):	##doesn't work reliably
	rr = game.random_range(3,5)
	pc = game.party[rr]
	pc.critter_kill_by_effect()
	game.particles( "sp-Slay Living", pc )
	game.particles( "ef-MinoCloud", pc )
	return


def pick_random_six( attachee, triggerer ):	##doesn't work reliably
	rr = game.random_range(3,6)
	pc = game.party[rr]
	pc.critter_kill_by_effect()
	game.particles( "sp-Slay Living", pc )
	game.particles( "ef-MinoCloud", pc )
	return


def pick_random_seven( attachee, triggerer ):	##doesn't work reliably
	rr = game.random_range(3,7)
	pc = game.party[rr]
	pc.critter_kill_by_effect()
	game.particles( "sp-Slay Living", pc )
	game.particles( "ef-MinoCloud", pc )
	return


def pick_random_eight( attachee, triggerer ):	##doesn't work reliably
	rr = game.random_range(3,8)
	pc = game.party[rr]
	pc.critter_kill_by_effect()
	game.particles( "sp-Slay Living", pc )
	game.particles( "ef-MinoCloud", pc )
	return


def kill_pc_3( attachee, triggerer ):
	pc = game.party[2]
	if (pc.type == obj_t_pc):
		pc.critter_kill_by_effect()
		game.particles( "sp-Slay Living", pc )
		game.particles( "ef-MinoCloud", pc )
		pc.critter_flag_unset(OCF_PARALYZED)
	else:
		kill_pc_4( attachee, triggerer )
	return 1


def kill_pc_4( attachee, triggerer ):
	pc = game.party[3]
	if (pc.type == obj_t_pc):
		pc.critter_kill_by_effect()
		game.particles( "sp-Slay Living", pc )
		game.particles( "ef-MinoCloud", pc )
		pc.critter_flag_unset(OCF_PARALYZED)
	else:
		kill_pc_5( attachee, triggerer )
	return 1


def kill_pc_5( attachee, triggerer ):
	pc = game.party[4]
	if (pc.type == obj_t_pc):
		pc.critter_kill_by_effect()
		game.particles( "sp-Slay Living", pc )
		game.particles( "ef-MinoCloud", pc )
		pc.critter_flag_unset(OCF_PARALYZED)
	else:
		kill_pc_6( attachee, triggerer )
	return 1


def kill_pc_6( attachee, triggerer ):
	pc = game.party[5]
	if (pc.type == obj_t_pc):
		pc.critter_kill_by_effect()
		game.particles( "sp-Slay Living", pc )
		game.particles( "ef-MinoCloud", pc )
		pc.critter_flag_unset(OCF_PARALYZED)
	else:
		kill_pc_7( attachee, triggerer )
	return 1


def kill_pc_7( attachee, triggerer ):
	pc = game.party[6]
	if (pc.type == obj_t_pc):
		pc.critter_kill_by_effect()
		game.particles( "sp-Slay Living", pc )
		game.particles( "ef-MinoCloud", pc )
		pc.critter_flag_unset(OCF_PARALYZED)
	else:
		kill_pc_3( attachee, triggerer )
	return 1


def dom_mon( attachee, triggerer ):
	leader = game.party[0]
	game.particles( "sp-Charm Monster", leader )
	game.particles( "swirled gas", leader )
	game.particles( "cast-Enchantment-cast", attachee )
	for dude in game.party:
		dude.condition_add_with_args("Paralyzed",4,0)

	kenan = find_npc_near(attachee,8804)
	sharar = find_npc_near(attachee,8806)
	gadham = find_npc_near(attachee,8807)
	abaddon = find_npc_near(attachee,8808)
	gershom = find_npc_near(attachee,8810)
	kenan.turn_towards(attachee)
	sharar.turn_towards(attachee)
	gadham.turn_towards(attachee)
	abaddon.turn_towards(attachee)
	gershom.turn_towards(attachee)
	return RUN_DEFAULT


def dom_mon_end( attachee, triggerer ):
	pc = game.party[0]
	game.particles( "Fizzle", pc )
	game.particles( "Gaseous Swirly", pc )
	game.particles( "Fizzle", attachee )
	return RUN_DEFAULT


def daniel_see_tarah( attachee, triggerer ):
	tarah = find_npc_near(attachee,8805)
	daniel = find_npc_near(attachee,8720)
	game.particles( "cast-Conjuration-cast", tarah )
	game.particles( "sp-Dimension Door", daniel )
	daniel.move( location_from_axis( 507, 587 ) )
	daniel.turn_towards(tarah)
	tarah.turn_towards(daniel)
	return


def kella_see_tarah( attachee, triggerer ):
	tarah = find_npc_near(attachee,8805)
	kella = find_npc_near(attachee,8070)
	game.particles( "cast-Conjuration-cast", tarah )
	game.particles( "sp-Dimension Door", kella )
	kella.move( location_from_axis( 506, 588 ) )
	kella.turn_towards(tarah)
	tarah.turn_towards(kella)
	return


def meleny_see_tarah( attachee, triggerer ):
	tarah = find_npc_near(attachee,8805)
	meleny = find_npc_near(attachee,8015)
	game.particles( "cast-Conjuration-cast", tarah )
	game.particles( "sp-Dimension Door", meleny )
	meleny.move( location_from_axis( 506, 588 ) )
	meleny.turn_towards(tarah)
	tarah.turn_towards(meleny)
	return


def fruella_see_tarah( attachee, triggerer ):
	tarah = find_npc_near(attachee,8805)
	fruella = find_npc_near(attachee,8067)
	game.particles( "cast-Conjuration-cast", tarah )
	game.particles( "sp-Dimension Door", fruella )
	fruella.move( location_from_axis( 506, 588 ) )
	fruella.turn_towards(tarah)
	tarah.turn_towards(fruella)
	return


def riana_see_tarah( attachee, triggerer ):
	tarah = find_npc_near(attachee,8805)
	riana = find_npc_near(attachee,8058)
	game.particles( "cast-Conjuration-cast", tarah )
	game.particles( "sp-Dimension Door", riana )
	riana.move( location_from_axis( 506, 588 ) )
	riana.turn_towards(tarah)
	tarah.turn_towards(riana)
	return


def serena_see_tarah( attachee, triggerer ):
	tarah = find_npc_near(attachee,8805)
	serena = find_npc_near(attachee,8056)
	game.particles( "cast-Conjuration-cast", tarah )
	game.particles( "sp-Dimension Door", serena )
	serena.move( location_from_axis( 506, 588 ) )
	serena.turn_towards(tarah)
	tarah.turn_towards(serena)
	return


def pishella_see_tarah( attachee, triggerer ):
	tarah = find_npc_near(attachee,8805)
	pishella = find_npc_near(attachee,8069)
	game.particles( "cast-Conjuration-cast", tarah )
	game.particles( "sp-Dimension Door", pishella )
	pishella.move( location_from_axis( 506, 588 ) )
	pishella.turn_towards(tarah)
	tarah.turn_towards(pishella)
	return

def pc2_see_tarah( attachee, triggerer ):
	tarah = find_npc_near(attachee,8805)
	pc2 = game.party[1]
	game.particles( "cast-Conjuration-cast", tarah )
	game.particles( "sp-Dimension Door", pc2 )
	pc2.move( location_from_axis( 506, 588 ) )
	pc2.turn_towards(tarah)
	tarah.turn_towards(pc2)
	return


def start_fight(attachee, triggerer):
	attachee.attack(triggerer)
	kenan = find_npc_near(attachee, 8804)
	sharar = find_npc_near(attachee, 8806)
	gadham = find_npc_near(attachee, 8807)
	abaddon = find_npc_near(attachee, 8808)
	gershom = find_npc_near(attachee, 8810)
	persis = find_npc_near(attachee, 8811)
	kenan.attack(triggerer)
	sharar.attack(triggerer)
	gadham.attack(triggerer)
	abaddon.attack(triggerer)
	gershom.attack(triggerer)
	persis.attack(triggerer)
	return