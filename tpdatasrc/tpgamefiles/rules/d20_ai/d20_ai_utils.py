from toee import *

# Not exactly sure of the purpose of this check, but it's
# used in considerTarget.
def isBusted(obj):
	flags = obj.obj_get_int(obj_f_flags)

	if not (flags & OF_DESTROYED):
		typ = obj.obj_get_int(obj_f_type)
		if typ == obj_t_portal:
			return obj.obj_get_int(obj_f_portal_flags) & OPF_BUSTED
		elif typ == obj_t_container:
			return obj.obj_get_int(obj_f_container_flags) & OCOF_BUSTED
		elif typ == obj_t_scenery:
			return obj.obj_get_int(obj_f_scenery_flags) & OSF_BUSTED
		elif typ == obj_t_trap:
			return obj.obj_get_int(obj_f_trap_flags) & OTF_BUSTED

	return 0

# Check if `target` is charmed by `critter`
def isCharmedBy(critter, target):
	if target.d20_query(Q_Critter_Is_Charmed):
		charmer = target.d20_query_get_data(Q_Critter_Is_Charmed, 0)
		return charmer == critter

	return 0

def getLivingPartyMemberCount():
	count = 0
	for pc in game.party:
		if not pc.is_dead_or_destroyed():
			count += 1

	return count

def partyHasNonCharmedLivingMembers():
	count = 0
	for pc in game.party:
		if not pc.d20_query(Q_Critter_Is_Charmed):
			if not pc.is_dead_or_destroyed():
				count += 1

	# TODO: this seems wrong, but it's what the original says
	return count == 0

def targetIsPcPartyNotDead(target):
	if target.obj_get_int(obj_f_type) == obj_t_pc:
		if partyHasNonCharmedLivingMembers():
			return True
		# TODO: this <= also seems backwards
		if getLivingPartyMemberCount() <= 1:
			return True

	return False

