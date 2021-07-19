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
		if missing_stub("NpcAiListFindAlly(attacker, target)"): return 0
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

def find_suitable_target(attacker, aiSearchingTgt):
	if aiSearchingTgt: return OBJ_HANDLE_NULL

	aiSearchingTgt = 1
	objToTurnTowards = OBJ_HANDLE_NULL

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

			friendFocus = GetFriendsCombatFocus(attacker, target, leader)
			if friendFocus != OBJ_HANDLE_NULL:
				kos_candidate = friendFocus
				break

		deadFriend = GetTargetFromDeadFriend(attacker, target)
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

	if not missing_stub('ExecuteObjectScript(target, attacker, WillKos)'):
		return 0

	if missing_stub('AiListFind(attacker, target, 1)'):
		return 0

	aaifs = missing_stub('GetAiFightStatus(attacker)')
	if aaifs.flags == AIFS_SURRENDERED and aaifs.target == target:
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
				rec = will_kos(attacker, charmer, aiFindingTarget)
				if rec: return rec

		# check AI params hostility threshold
		if target.type == obj_t_pc:
			aiParams = missing_stub('attacker.get_ai_params')
			reaction = attacker.reaction_get(target)
			if reaction <= aiParams.hostility_threshold)::
				return 2

			return 0

	if target.type == obj_t_npc:
		return 0

	taifs = missing_stub('GetAiFightStatus(target)')
	if taifs.flags != AIFS_FIGHTING or taifs.target != OBJ_HANDLE_NULL:
		return 0
	if attacker.allegiance_strength(taifs.target) == 0:
		return 0

	leader = attacker.leader_get()
	if attacker.cannot_hate(target, leader):
		return 4

	return 0

def execute_strategy(obj, target):

    return None # this causes the engine to ignore the result; return 0 / 1 to make it work
