from toee import *
import tpdp
import tpai
from d20_ai_utils import *

debug_enabled = False

def debug_print(*args):
    if debug_enabled:
        for arg in args:
            print arg,
    return



def missing_stub(msg):
	print msg
	return 0

def cannot_hate(obj, tgt, leader):
	OHN = OBJ_HANDLE_NULL
	if obj == OHN:
		return 0
	spell_flags = obj.obj_get_int(obj_f_spell_flags)
	if (spell_flags & 0x8000000) and obj.leader != OHN:
		return 0
	if tgt == OHN or not tgt.is_critter():
		return 0
	if leader != OHN and tgt.leader == leader:
		return 4
	if (obj.allegiance_shared(tgt)):
		return 3
	if obj.d20_query_has_condition("sp-Sanctuary Save Failed") == 0 or tgt.d20_query_has_condition("sp-Sanctuary") == 0:
		return 0
	tgt_sanctuary_handle = tgt.d20_query_get_obj(Q_Critter_Has_Condition, tpdp.hash("sp-Sanctuary"))
	sanctuary_handle     = obj.d20_query_get_obj(Q_Critter_Has_Condition, tpdp.hash("sp-Sanctuary Save Failed"))
	if sanctuary_handle == tgt_sanctuary_handle:
		return 5
	return 0

# aiSearchingTgt is used to avoid infinite looping since
# consider_target calls will_kos which calls consider_target.
# In the C(++), this is a global variable. Trying to avoid that.
def consider_target(attacker, target, aiSearchingTgt=False):
	if attacker == target: return 0
	# if aiSearchingTgt == False:
	# 	debug_print("consider_target: " + str(attacker) + " considering " + str(target))
	attacker_flags = target.obj_get_int(obj_f_critter_flags)
	ignore_flags = OF_INVULNERABLE | OF_DONTDRAW | OF_OFF | OF_DESTROYED
	target_flags = target.obj_get_int(obj_f_flags)

	if target_flags & ignore_flags: 
		debug_print("consider_target:ignore_flags")
		return 0

	a_leader = attacker.leader_get()

	if not target.is_critter():
		if isBusted(target): 
			debug_print("consider_target: is busted")
			return 0
	else:
		if aiListFind(attacker, target, AI_LIST_ALLY): 	
			debug_print("consider_target:ai list ally")
			return 0
		if target == OBJ_HANDLE_NULL: return 0
		if target.is_dead_or_destroyed(): return 0
		if target.is_unconscious():
			if attacker_flags & OCF_NON_LETHAL_COMBAT: return 0
			if target != find_suitable_target(attacker, aiSearchingTgt): return 0

		if target == a_leader: return 0

		t_leader = target.leader_get()
		if t_leader != OBJ_HANDLE_NULL and t_leader == a_leader:
			debug_print("consider_target:same leader")
			return 0

		if isCharmedBy(target, attacker):
			return targetIsPcPartyNotDead(target)

		if target.is_friendly(attacker): 
			debug_print("consider tgt: is friendly: " + str(target) + "\n")
			return 0

		if target.distance_to(attacker) > 125.0: 
			debug_print("consider_target:too far")
			return 0

	if a_leader != OBJ_HANDLE_NULL:
		if a_leader.distance_to(target) > 20:
			if attacker.distance_to(target) > 15.0:
				debug_print("consider tgt: too far from leader")
				return 0
	debug_print("consider_target: Return true")
	return 1

def getFriendsCombatFocus(obj, friend, leader, aiSearchingTgt = False):
	if friend.type != obj_t_npc:
		return OBJ_HANDLE_NULL

	# moved this check here since it's gating anyway; used to be after consider_target
	if obj.allegiance_strength(friend) == 0:
		return OBJ_HANDLE_NULL

	aifs = AiFightStatus(friend)
	tgt = aifs.target
	if tgt == OBJ_HANDLE_NULL:
		return tgt
	if not aifs.status in [AIFS_FIGHTING, AIFS_FLEEING, AIFS_SURRENDERED]:
		return OBJ_HANDLE_NULL

	if cannot_hate(obj, tgt, leader):
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
	if friend == OBJ_HANDLE_NULL: return OBJ_HANDLE_NULL
	if not friend.is_dead_or_destroyed(): return OBJ_HANDLE_NULL
	if friend.type != obj_t_npc: return OBJ_HANDLE_NULL
	if aiListFind(obj, friend, AI_LIST_ALLY): return OBJ_HANDLE_NULL # is this a bug?
	
	if obj.allegiance_strength(friend) == 0:
		return OBJ_HANDLE_NULL
	tgt = friend.obj_get_obj(obj_f_npc_combat_focus)
	if not consider_target(obj, tgt, True): 
		return OBJ_HANDLE_NULL
	return tgt

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
		if not (dude in game.party): # added so that party members are prioritized over non-party NPCs (e.g. bystanders or whatever)
			dist += 100.0
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

	if target.type != obj_t_npc:
		return 0

	taifs = AiFightStatus(target)
	if taifs.status != AIFS_FIGHTING or taifs.target != OBJ_HANDLE_NULL:
		return 0
	if attacker.allegiance_strength(taifs.target) == 0:
		return 0

	leader = attacker.leader_get()
	if cannot_hate(attacker, target, leader):
		return 4

	return 0

