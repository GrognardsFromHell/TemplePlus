from toee import *

# Common utilities for banishment effects

# Tests if a creature is on its home plane. Generally, spells only allow
# banishment if the caster is on their home plane.
#
# This is the override for KotB. I haven't played all the way through, but
# I'm assuming it takes place entirely on the prime material plane, so that
# no special handling is necessary. This function should be modified if this
# is not the case.
def IsOnHomePlane(critter):
	is_extra_sub = critter.is_category_subtype(mc_subtype_extraplanar)
	is_extra_flag = critter.npc_flags_get() & ONF_EXTRAPLANAR
	return not (is_extra_sub or is_extra_flag)

# This tests if a critter can be banished. Strictly speaking, most spells
# only state that this follows the results of `IsOnHomePlane`. However, most
# scripts in the game also check for summoned creatures, so that is added
# here.
def IsBanishable(critter):
	summoned = critter.d20_query_has_spell_condition(sp_Summoned)
	return summoned or not IsOnHomePlane(critter)
