from templeplus.pymod import PythonModifier, BasicPyMod
from toee import *
import tpdp
from utilities import *

# Performs a typical countdown and remove condition. Parameter 0 can be used
# to indicate which argument has the duration.
def CountdownRemoveSelf(obj, args, evt_obj):
	dur_index = args.get_param(0, default = 0)
	ticked = evt_obj.data1
	new_dur = args.get_arg(dur_index) - ticked
	if new_dur < 0:
		args.condition_remove()
	else:
		args.set_arg(dur_index, new_dur)

	return 0

# Ends particles whose particle id is stored on one of the condition
# arguments. By default, this is argument 1, but parameter 0 allows specifying
# a different argument index.
def EndParticles(obj, args, evt_obj):
	part_index = args.get_param(0, default = 1)
	skip_end = args.get_param(1)

	part_id = args.get_arg(part_index)
	if part_id != 0:
		game.particles_end(part_id)
		if not skip_end:
			game.particles(args.get_cond_name() + "-END", obj)
		args.set_arg(part_index, 0)

	return 0

# Implements one condition being removed by another, like Neutralize Poison
# curing poison. The removing conditions are stored by hash in the parameters,
# and any number of conditions is supported by a single hook (so long as a
# tuple that large is possible).
def CheckRemovedBy(obj, args, evt_obj):
	ix = 0
	key = args.get_param(ix)
	while key != 0:
		if evt_obj.is_modifier_hash(key):
			args.condition_remove()
			break
		ix += 1
		key = args.get_param(ix)

	return 0

# Implements one condition being removed by another, but sets the flag that
# makes the removing effect not actually be added. So the effects cancel each
# other out rather than one taking the place of the other.
#
# This is used for e.g. Lesser Restoration, where it dispels any number of
# ability penalties, but if it does so it does not heal ability damage. The
# healing is triggerd by the Lesser Restoration condition being actually
# added, so by preempting that it will only cancel penalties.
#
# Like CheckRemovedBy, any number of conditions are supported by providing as
# many parameters. But the first parameter controls whether `return_val` is
# checked. If it is, and `return_val` is 0, this condition will not be
# removed. This implements a spell cancelling out exactly one condition,
# rather than all relevant conditions.
def CheckCancelOut(obj, args, evt_obj):
	# if param 0 is true, and the removing condition has been used up, do
	# nothing
	if args.get_param(0) and not evt_obj.return_val: return 0

	ix = 1
	key = args.get_param(ix)
	while key != 0:
		if evt_obj.is_modifier_hash(key):
			args.condition_remove()
			evt_obj.return_val = 0 # don't add condition
			break

		ix += 1
		key = args.get_param(ix)

	return 0

# Implements one condition preventing others. For instance, Neutralize
# Poison prevents poison conditions from being added.
#
# Like the above hooks, any number of conditions are supported by providing
# multiple parameters. The parameters should be the hash of the conditions
# to be prevented.
def CheckPrevent(obj, args, evt_obj):
	ix = 0
	key = args.get_param(ix)
	while key != 0:
		if evt_obj.is_modifier_hash(key):
			evt_obj.return_val = 0 # don't add condition
			break

		ix += 1
		key = args.get_param(ix)

	return 0

# Simply removes the condition
def Remove(obj, args, evt_obj):
	args.condition_remove()
	return 0

# Shows an effect tooltip using the hash key from parameter 0
def KeyedEffectTooltip(critter, args, evt_obj):
	key = args.get_param(0)
	if key == 0: return 0

	distinct = args.get_param(1)

	if distinct:
		evt_obj.append_distinct(key, -2, "")
	else:
		evt_obj.append(key, -2, "")

	return 0

def AbilityPenaltyHook(critter, args, evt_obj):
	pen_type = args.get_param(0)
	pen_index = args.get_param(1)
	mesline = args.get_param(2)
	pen_amount = args.get_arg(pen_index)

	evt_obj.bonus_list.add(-pen_amount, pen_type, mesline)
	return 0

# This hook adds particles for the condition's name to the critter the
# condition affects. Typically this would be used as a ConditionAdd hook.
#
# The particle id is stored in an argument. By default this is arg 1, but if
# param 0 is specified, it is used as the argument index for the particle id.
# A negative index results in the particle id not being stored (not
# recommended for permanent particles).
def BeginParticlesHook(critter, args, evt_obj):
	part_idx = args.get_param(0, default = 1)

	part_id = game.particles(args.get_cond_name(), critter)
	if (part_idx >= 0):
		args.set_arg(part_idx, part_id)

	return 0

# Helper defining functions for building a standalone condition not associated
# to a spell.
class CondFunctions(BasicPyMod):
	# Adds a hook to begin particles when the condition is added. The index is
	# used to store the particle id, and should be matched to the one in
	# `AddEndParticles`.
	def AddBeginParticles(self, index = 1):
		self.add_hook(ET_OnConditionAdd, EK_NONE, BeginParticlesHook, (index,))

	# Adds a hook that ends particles when the condition is removed. The default
	# is for the particle id to be stored in slot 1, but that can be overridden.
	#
	# the skip_end parameter controls whether the hook should try to play ending
	# particles if the persistent particles are actually in place.
	def AddEndParticles(self, index = 1, skip_end = 0):
		params = (index,skip_end)
		self.add_hook(ET_OnConditionRemove, EK_NONE, EndParticles, params)

	# Adds a hook that removes this condition of one of the other specified
	# conditions is added. The removing conditions are specified by name.
	def AddRemovedBy(self, *cond_names):
		if len(cond_names) <= 0: return

		keys = tuple(tpdp.hash(cond_name) for cond_name in cond_names)

		self.add_hook(ET_OnConditionAddPre, EK_NONE, CheckRemovedBy, keys)

	# Adds a hook that causes this condition to be 'canceled out' by the
	# specified conditions. This condition gets removed, but the cancelling
	# condition also does not get added. `single` controls whether the specified
	# conditions can cancel out only a single of these conditions, or
	# arbitrarily many.
	def AddCancelOut(self, single, *cond_names):
		if len(cond_names) <= 0: return

		keys = [tpdp.hash(cond_name) for cond_name in cond_names]
		keys.insert(0, 1 if single else 0)
		params = tuple(keys)

		self.add_hook(ET_OnConditionAddPre, EK_NONE, CheckCancelOut, params)

	# Remove condition on creature death.
	def AddRemoveOnDeath(self):
		self.add_hook(ET_OnD20Signal, EK_S_Killed, Remove, ())

	# Adds a hook that shows the effect tooltip with the given name.
	#
	# `distinct = True` means that only one copy of the tooltip will be shown
	# even if the creature has the condition multiple times.
	def AddEffectTooltip(self, name, distinct = False):
		params = (tpdp.hash(name), distinct)
		
		self.add_hook(ET_OnGetEffectTooltip, EK_NONE, KeyedEffectTooltip, params)

	def AddCountdownRemove(self):
		self.add_hook(ET_OnBeginRound, EK_NONE, CountdownRemoveSelf, ())

	# Adds a hook for ability penalties
	#
	# The first argument specifies a bonus number to use, which determines which
	# penalties can stack with one another.
	#
	# The second argument determines whether the penalty is 'soft', which
	# prevents it from reducing a score below 1. This is dependent on the type,
	# so the exact bonus type may be different from the specified one in that
	# case.
	#
	# The third argument is a mesline number for the description.
	#
	# Remaining arguments should either be a sequence of numbers or tuples. In
	# the first case, the numbers specify which abilities are penalized, with
	# the penalty pulling from arguments starting at 2. In the second case, the
	# tuple should contain an ability number and an argument index.
	def AddAbilityPenalties(self, ty, soft, mes, *args):
		if soft: ty |= 0x10000

		i = 2
		for arg in args:
			if isinstance(arg, tuple):
				(ability, index) = arg
			else:
				ability = arg
				index = i
			key = EK_STAT_STRENGTH + ability
			params = (ty, index, mes)
			self.add_hook(ET_OnAbilityScoreLevel, key, AbilityPenaltyHook, params)
			i += 1

# This is a class helper for standalone conditions not associated to a spell.
# As such, it does not store a spell id, and does not have dispel hooks and
# the like. Generally this should be useful for conditions caused by things
# like extraordinary abilities, or instantaneous spells with prolonged
# effects.
#
# By default these modifiers have 3 arguments. The convention for these is:
#
#  0: duration
#  1: particle id
#  2: misc
#
# However, the conventions of some of these can be overridden if desired.
#
# Also, the default for these is to not prevent duplicates.
class CondPythonModifier(CondFunctions):
	def __init__(self, name, args = 3, preventDuplicates = False, permanent = False):
		super(CondFunctions, self).__init__(name, args, preventDuplicates)

		if not permanent:
			self.AddRemoveOnDeath()
			self.AddCountdownRemove()

		self.AddEffectTooltip(name)

