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

	part_id = args.get_arg(part_index)
	if part_id != 0:
		game.particles_end(part_id)
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

# Helper defining functions for building a standalone condition not associated
# to a spell.
class CondFunctions(BasicPyMod):
	# Adds a hook that ends particles when the condition is removed. The default
	# is for the particle id to be stored in slot 1, but that can be overridden.
	def AddEndParticlesHook(self, index = 1):
		self.add_hook(ET_OnConditionRemove, EK_NONE, EndParticles, (index,))

	# Adds a hook that removes this condition of one of the other specified
	# conditions is added. The removing conditions are specified by name.
	def AddRemovedByHook(self, *cond_names):
		if len(cond_names) <= 0: return

		keys = tuple(tpdp.hash(cond_name) for cond_name in cond_names)

		self.add_hook(ET_OnConditionAddPre, EK_NONE, CheckRemovedBy, keys)

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

