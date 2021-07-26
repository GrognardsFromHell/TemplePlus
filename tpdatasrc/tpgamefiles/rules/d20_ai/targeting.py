from toee import *
import tpai
from d20_ai_utils import *

def missing_stub(msg):
	print msg
	return 0

# aiSearchingTgt is used to avoid infinite looping since
# consider_target calls will_kos which calls consider_target.
# In the C(++), this is a global variable. Trying to avoid that.
def consider_target(attacker, target, aiSearchingTgt=False):
	if attacker == target: return 0

	attacker_flags = target.obj_get_int(obj_f_critter_flags)
	ignore_flags = OF_INVULNERABLE | OF_DONTDRAW | OF_OFF | OF_DESTROYED
	target_flags = target.obj_get_int(obj_f_critter_flags)

	if target_flags & ignore_flags: return 0

	a_leader = attacker.leader_get()

	if not target.is_critter():
		if isBusted(target): return 0
	else:
		if aiListFind(attacker, target, AI_LIST_ALLY): return 0
		if target == OBJ_HANDLE_NULL: return 0
		if target.is_dead_or_destroyed(): return 0
		if target.is_unconscious():
			if attacker_flags & OCF_NON_LETHAL_COMBAT: return 0
			if target != find_suitable_target(attacker, aiSearchingTgt): return 0

		if target == a_leader: return 0

		t_leader = target.leader_get()
		if t_leader != OBJ_HANDLE_NULL and t_leader == a_leader: return 0

		if isCharmedBy(target, attacker):
			return targetIsPcPartyNotDead(target)

		if target.is_friendly(attacker): return 0

		if target.distance_to(attacker) > 125.0: return 0

	if a_leader != OBJ_HANDLE_NULL:
		if a_leader.distance_to(target) > 20:
			if attacker.distance_to(target) > 15.0:
				return 0

	return 1

def getFriendsCombatFocus(obj, friend, leader, aiSearchingTgt = False):
	if friend.type != obj_t_npc:
		return OBJ_HANDLE_NULL

	# moved this check here since it's gating anyway; used to be after consider_target
	if obj.allegiance_strength(friend) == 0:
		return OBJ_HANDLE_NULL

	aifs = tpai.AiFightStatus(friend)
	tgt = aifs.target
	if tgt == OBJ_HANDLE_NULL:
		return tgt
	if not aifs.status in [AIFS_FIGHTING, AIFS_FLEEING, AIFS_SURRENDERED]:
		return OBJ_HANDLE_NULL

	if obj.cannot_hate(tgt, leader):
		return OBJ_HANDLE_NULL

	if not consider_target(obj, tgt, aiSearchingTgt):
		return OBJ_HANDLE_NULL

	if not detect(obj, tgt):
		return OBJ_HANDLE_NULL

	# new in Temple+ : check pathfinding short distances (to simulate sensing nearby critters)
	pf_flags = PQF_HAS_CRITTER | PQF_IGNORE_CRITTERS | PQF_800 \
		| PQF_TARGET_OBJ | PQF_ADJUST_RADIUS | PQF_ADJ_RADIUS_REQUIRE_LOS \
		| PQF_DONT_USE_PATHNODES | PQF_A_STAR_TIME_CAPPED
	
	# TODO
	missing_stub("tpconfig.alertAiThroughDoors")
	# if tpconfig.alertAiThroughDoors:
	# 	pf_flags |= PQF_DOORS_ARE_BLOCKING

	path_len = obj.can_find_path_to_obj(tgt, pf_flags)
	PATH_LEN_MAX = 40
	if path_len < PATH_LEN_MAX:
		return tgt
	
	# Hmm, this section skips the detection check, might need to be revisited
	if not obj in game.party:
		for pc in game.party:
			path_len = obj.can_find_path_to_obj(tgt, pf_flags)
			if path_len < PATH_LEN_MAX:
				return pc

	return OBJ_HANDLE_NULL

def getTargetFromDeadFriend(obj, friend):
	missing_stub("getTargetFromDeadFriend")
	return OBJ_HANDLE_NULL

def find_suitable_target(attacker, aiSearchingTgt):
	if aiSearchingTgt: return OBJ_HANDLE_NULL

	aiSearchingTgt = 1
	turn_towards = OBJ_HANDLE_NULL

	leader = attacker.leader_get()

	candidates = game.obj_list_range(attacker.location, 18, OLC_CRITTERS)
	pairs = []

	for dude in candidates:
		if dude == attacker: continue
		dist = attacker.location_full.distance_to(dude.location_full)
		if dude.is_unconscious():
			dist += 1000.0
		pairs.append((dude,dist))

	pairs.sort(key = lambda p: p[1])

	kos_candidate = OBJ_HANDLE_NULL

	for target, dist in pairs:
		if detect(attacker, target):
			if target.is_category_type(obj_t_pc):
				turn_towards = target
			if will_kos(attacker, target, aiSearchingTgt):
				kos_candidate = target
				break

			friendFocus = getFriendsCombatFocus(attacker, target, leader, aiSearchingTgt)
			if friendFocus != OBJ_HANDLE_NULL:
				kos_candidate = friendFocus
				break

		deadFriend = getTargetFromDeadFriend(attacker, target)
		if detect(attacker, deadFriend):
			kos_candidate = deadFriend
			break

	if kos_candidate == OBJ_HANDLE_NULL:
		if turn_towards != OBJ_HANDLE_NULL:
			attacker.turn_towards(turn_towards)

	return kos_candidate

# Common check during find_suitable_target
def detect(attacker, target):
	if target == OBJ_HANDLE_NULL: return False

	unconcealed = isUnconcealed(target)

	result = attacker.can_hear(target, unconcealed)
	result = result or not attacker.has_los(target)

	return result

def will_kos(attacker, target, aiSearchingTgt):
	if not consider_target(attacker, target, aiSearchingTgt):
		return 0

	leader = getLeaderForNPC(attacker)

	if not attacker.object_script_execute(target, san_will_kos):
		return 0

	if aiListFind(attacker, target, AI_LIST_ALLY): # friend list
		return 0

	aaifs = AiFightStatus(attacker)
	if aaifs.status == AIFS_SURRENDERED and aaifs.target == target:
		return 0

	isInParty = attacker in game.party

	if leader != OBJ_HANDLE_NULL and not isInParty:
		npcFlags = attacker.npc_flags_get()
		if (npcFlags & ONF_KOS) and not (npcFlags & ONF_KOS_OVERRIDE):
			allegiance = attacker.allegiance_shared(target)
			faction = target.type == obj_t_pc or not hasNullFaction(target)

			if allegiance and faction:
				return 1

			if target.d20_query(Q_Critter_Is_Charmed):
				charmer = target.d20_query_get_obj(Q_Critter_Is_Charmed)
				rec = will_kos(attacker, charmer, aiSearchingTgt)
				if rec: return rec

		# check AI params hostility threshold
		if target.type == obj_t_pc:
			aiParams = tpai.AiParams(attacker)
			reaction = attacker.reaction_get(target)
			if reaction <= aiParams.hostility_threshold:
				return 2

			return 0

	if target.type == obj_t_npc:
		return 0

	taifs = AiFightStatus(target)
	if taifs.status != AIFS_FIGHTING or taifs.target != OBJ_HANDLE_NULL:
		return 0
	if attacker.allegiance_strength(taifs.target) == 0:
		return 0

	leader = attacker.leader_get()
	if attacker.cannot_hate(target, leader):
		return 4

	return 0

def execute_strategy(obj, target):

	return None # this causes the engine to ignore the result; return 0 / 1 to make it work
