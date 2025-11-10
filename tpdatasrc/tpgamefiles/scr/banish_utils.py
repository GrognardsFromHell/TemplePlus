from toee import *

# Common utilities for banishment effects

air_node = 5081
earth_node = 5082
fire_node = 5083
water_node = 5084

nodes = set([air_node, earth_node, fire_node, water_node])

# Tests if a creature is on its home plane. Generally, spells only allow
# banishment if the caster is on their home plane.
#
# This is specific to the ToEE module, as it accounts for the elemental
# nodes. These are treated as pocket planes that no creatures are native to
# (so banishment never works).
def IsOnHomePlane(critter):
	if critter.map in nodes: return False

	# all other maps are (I think) prime material plane, so extraplanar flags
	# apply, either monster subtype or npc flags.
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
