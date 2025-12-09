import tpdp

class PythonModifier(tpdp.ModifierSpec):
	def AddHook(self, eventType, eventKey, callbackFcn, argsTuple ):
		self.add_hook(eventType, eventKey, callbackFcn, argsTuple)
	def ReplaceHook(self, ix, eventType, eventKey, callback, params):
		# Replaces a hook with a python hook.
		#
		# Should only be used to replace hooks already in place from
		# a modifier created with `ExtendExisting`. Trying to replace a
		# python hook will crash.
		self.replace_hook(ix, eventType, eventKey, callback, params)
	def ExtendExisting(self, condName):
		self.extend_existing(condName)
	def AddItemForceRemoveHandler(self): # in charge of backing up condition args
		self.add_item_force_remove_callback()
	def MapToFeat(self, feat_enum, feat_list_max = -1, feat_cond_arg2 = 0):
		# Feats are mapped to Modifier + arg
		# The associated Modifier is instantiated with 2 args:
		# arg[0] = feat enum
		# arg[1] = feat_cond_arg2 - usually used for "feat lists" to denote specialized type, such as weapon_focus_X or skill_focus_X
		# For feat lists, the vanilla engine was using range(feat_enum, feat_list_max), 
		# but for Temple+ feats we just use a normal dictionary.
		# So just loop over all sub-feats + their desired arg[1] and leave feat_list_max = -1
		self.add_to_feat_dict(feat_enum, feat_list_max, feat_cond_arg2)
	# Spell related standard hooks
	def AddSpellCountdownStandardHook(self):
		# adds an ET_OnBeginRound handler that (normally) does:
		# If countdown expired: (<0)
		#   1. Float text "Spell Expired"
		#   2. RemoveSpell() (has case-by-case handlers for Spell_End; Temple+ adds generic handling for wall spells here)
		#   3. RemoveSpellMod()
		# Else:
		#   Decrement count, update spell packet duration
		self.add_spell_countdown_standard()    
	def AddAoESpellEndStandardHook(self): 
		# adds a EK_S_Spell_End handler that:
		# 1. Ends particles for all spell objects
		# 2. RemoveSpellMod()
		self.add_aoe_spell_ender()
	def AddSpellDismissStandardHook(self):
		self.add_spell_dismiss_hook()
	def AddSpellDispellCheckHook(self):
		#Adds the standard dispell hook for the condition
		self.add_spell_dispell_check_hook()
	def AddSpellTouchAttackDischargeRadialMenuHook(self):
		#Adds the standard discharge radial menu
		self.add_spell_touch_attack_discharge_radial_menu_hook()
	
	def AddSpellTeleportPrepareStandard(self):
		# If caster not in party: remove_spell, remove_spell_mod
		# If attachee is not in party:
		#     if target_count <= 1: remove_spell(), remove_spell_mod()
		# else:
		#		removes attachee from target list
		# Note that this applies to spell objects! (possible Co8 mod trouble)
		self.add_spell_teleport_prepare_standard()
	def AddSpellTeleportReconnectStandard(self):
		# Does nothing... oops!
		# Is generally handled centrally in the spell system now
		self.add_spell_teleport_reconnect_standard()
	
	def AddSpellDispelCheckStandard(self):
		self.add_spell_dispel_check_standard()

class BasicPyMod(tpdp.ModifierSpec):
	def AddHook(self, eventType, eventKey, callbackFcn, argsTuple):
		self.add_hook(eventType, eventKey, callbackFcn, argsTuple)
	def ExtendExisting(self, condName):
		self.extend_existing(condName)

class FeatPyMod(BasicPyMod):
	def __init__(self, feat_enum, feat_cond_arg2 = 0, args = 2, preventDuplicate = True, feat_list_max = -1):
		self.add_to_feat_dict(feat_enum, feat_list_max, feat_cond_arg2)
