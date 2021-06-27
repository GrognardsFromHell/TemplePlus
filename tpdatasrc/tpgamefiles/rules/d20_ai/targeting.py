from toee import *
from d20_ai_utils import *

def missing_stub(msg):
	print msg
	return 0

def considerTarget(attacker, target):
	if attacker == target: return 0

	attacker_flags = target.obj_get_int(obj_f_critter_flags)
  ignore_flags = OF_INVULNERABLE | OF_DONTDRAW | OF_OFF | OF_DESTROYED
	target_flags = target.obj_get_int(obj_f_critter_flags)

	if target_flags & ignore_flags: return 0

	a_leader = attacker.leader_get()

	if not target.is_critter():
		if isBusted(target): return 0
	else:
		if missing_stub("NpcAiListFindAlly(attacker, target)"): return 0
		if target == OBJ_HANDLE_NULL: return 0
		if target.is_dead_or_destroyed(): return 0
		if target.is_unconscious():
			if attacker_flags & OCF_NON_LETHAL_COMBAT: return 0
			if missing_stub("target != attacker.find_suitable_target()"): return 0

		if target == a_leader: return 0

		t_leader = target.leader_get()
		if t_leader != OBJ_HANDLE_NULL and t_leader == a_leader: return 0

		if isCharmedBy(target, attacker):
			return TargetIsPcPartyNotDead(target)

		if target.is_friendly(attacker): return 0

		if target.distance_to(attacker) > 125.0: return 0

	if a_leader != OBJ_HANDLE_NULL:
		tile_delta = missing_stub("GetTileDeltaMax(a_leader, target)")
		if tile_delta > 20:
			if attacker.distance_to(target) > 15.0:
				return 0

	return 1

