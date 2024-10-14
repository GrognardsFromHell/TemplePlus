from toee import *
from scripts import OSF_IS_HEZROU_STENCH, set_spell_flag, get_spell_flags, is_spell_flag_set, unset_spell_flag

def OnBeginSpellCast( spell ):
	print "Hezrou Stench OnBeginSpellCast id= ", spell.id
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster ) # change to stinking cloud?

def	OnSpellEffect( spell ):
	print "Hezrou Stench OnSpellEffect"
	spell.dc = 100
	spell.duration = 100
	processStench(spell.caster, spell.id)
	spell.spell_end(spell.id)

def OnAreaOfEffectHit( spell ):
	print "Hezrou Stench OnAreaOfEffectHit"

def OnBeginRound( spell ):
	print "Hezrou Stench OnBeginRound"

def OnEndSpellCast( spell ):
	print "Hezrou Stench OnEndSpellCast"
	endStench(spell.caster)


#------------------------------------------------------------------------------
# Originally in stench.py
#------------------------------------------------------------------------------

OBJ_SPELL_STENCH = 6271
STENCH_DURATION = 100
EFFECT_VAR = obj_f_item_pad_i_1
COUNT_VAR = obj_f_item_pad_i_2
STATE_VAR = obj_f_critter_pad_i_5
STATE_UNPROCESSED = 0
STATE_UNAFFECTED = 1
STATE_NOTINSTENCH = 2
STATE_NAUSEA = 3
STATE_NAUSEA_HANGOVER = 4
STATE_NAUSEA_EXPIRED = 5	# Unused. (I felt that it was only fair to get further saving throws after nausea expires)
STATE_SICKNESS = 6
STATE_NOPROCESS = 7
STATE_NEUTRALISED = 8
CID = 0
COUNTER = 0
BASECASTERID = 1
casterIdNext = BASECASTERID


# Main stench processing routine.
def processStench(caster, spell_id):

	# Get caster ID, initialising if neccessary.
	casterId = getObjVarNibble(caster, STATE_VAR, CID)
	if (casterId == 0):
		global casterIdNext
		casterId = casterIdNext
		casterIdNext = casterIdNext + 1
		if (casterIdNext > 7):
			casterIdNext = BASECASTERID
		setObjVarNibble(caster, STATE_VAR, CID, casterId)

	# While caster is alive, concious and not destroyed or off.
	#print "checking caster... flags= ", caster.object_flags_get() & 0xff, " anded=", (caster.object_flags_get() & (OF_DESTROYED | OF_OFF))
	if (caster.stat_level_get(stat_hp_current) > -10 and (caster.object_flags_get() & (OF_DESTROYED | OF_OFF)) == 0):

		print "Starting stench processing for ", caster, " id=", casterId, " spell_id=", spell_id
		for critter in game.obj_list_vicinity(caster.location, OLC_CRITTERS):

			if (critter.stat_level_get(stat_hp_current) <= -10):
				setObjVarNibble(critter, STATE_VAR, casterId, STATE_NOPROCESS)

			# Get critter status and process appropriately.
			status = getObjVarNibble(critter, STATE_VAR, casterId)

			if (status == STATE_UNPROCESSED):
				# Unprocessed - process critter init, and assign state.
				if (critter.is_category_subtype(mc_subtype_demon) or critter.is_category_type(mc_type_elemental) or critter.is_category_type(mc_type_undead) or
					critter.stat_level_get(stat_level_monk) > 10 or critter.stat_level_get(stat_level_druid) > 8):
					# Unaffected critters. (demons, elementals, undead, classes immune to poison)
					print critter, " unaffected"
					status = STATE_UNAFFECTED
				elif has_necklace(critter):
					print critter, " unaffected"
					status = STATE_UNAFFECTED
				elif (inStenchArea(caster, critter)):
					# In stench area - attempt save, apply the stench and take ownership.
					status = attemptSave(caster, spell_id, critter, status)
					applyStenchEffect(casterId, spell_id, critter, status)
				else:
					# Not yet in stench area.
					print critter, " not yet in stench area"
					status = STATE_NOTINSTENCH

			elif (status == STATE_NOTINSTENCH):
				# Check cured count.
				curedCount = getObjVarNibble(critter, STATE_VAR, COUNTER)
				if (curedCount > 0):
					print critter, " has been cured, skipping round"
					setObjVarNibble(critter, STATE_VAR, COUNTER, curedCount - 1)
				# Check if now in stink area.
				elif (inStenchArea(caster, critter)):
					# In stink area - apply the stench effect.
					print critter, " now in stench area, attempting save"
					status = attemptSave(caster, spell_id, critter, status)
					applyStenchEffect(casterId, spell_id, critter, status)

			elif (status == STATE_SICKNESS):
				# Check if now out of stink area.
				if (not inStenchArea(caster, critter)):
					print critter, " now out of stench area, removing sickness"
					status = STATE_NOPROCESS
					removeStenchEffect(casterId, spell_id, critter, status, false)

			elif (status == STATE_NAUSEA):
				# Check if now out of stink area.
				if (not inStenchArea(caster, critter)):
					status = STATE_NAUSEA_HANGOVER
					hangoverCount = game.random_range(0, 3)
					stench_obj = getStenchObj(critter)
					if (stench_obj != OBJ_HANDLE_NULL):
						print critter, " now out of stench area, changing to nausea hangover, count= ", hangoverCount
						setObjVarNibble(stench_obj, EFFECT_VAR, casterId, status)
						setObjVarNibble(stench_obj, COUNT_VAR, casterId, hangoverCount)
					else:
						print "Error: Can't find spell object for nausea."
						status = STATE_NOTINSTENCH	# In case of error.

			elif (status == STATE_NAUSEA_HANGOVER):
				stench_obj = getStenchObj(critter)
				if (stench_obj != OBJ_HANDLE_NULL):
					# Check if now back in stink area.
					if (inStenchArea(caster, critter)):
						print critter, " now back in stench area, changing back to nausea"
						status = STATE_NAUSEA
						setObjVarNibble(stench_obj, EFFECT_VAR, casterId, status)
					else:
						# Maintain hangover.
						hangoverCount = getObjVarNibble(stench_obj, COUNT_VAR, casterId)
						print critter, " has nausea hangover, count= ", hangoverCount
						if (hangoverCount < 1):
							status = STATE_NOTINSTENCH
							removeStenchEffect(casterId, spell_id, critter, status, true)
						else:
							setObjVarNibble(stench_obj, COUNT_VAR, casterId, hangoverCount - 1)
				else:
					print "Error: Can't find spell object for hangover."
					status = STATE_NOTINSTENCH	# In case of error.
					
			else:
				continue
				
			# Store critter status.
			setObjVarNibble(critter, STATE_VAR, casterId, status)

		# Setup processing for next round.
		game.timevent_add(processStench, (caster, spell_id), 500)

	else:
		print "Caster died or expired, ending the stench."
		endStench(caster, spell_id)
	return


def inStenchArea(caster, critter):
	return (critter.distance_to(caster) < 10)


def attemptSave(caster, spell_id, critter, status):
	# In stink area - attempt saving throw.
	if (critter.saving_throw_spell(24, D20_Save_Fortitude, D20CO8_F_POISON, caster, spell_id)):
		# Passed saving throw, apply sickness.
		status = STATE_SICKNESS
		print critter, " is in stench, passed saving throw - applying sickness"
		critter.float_mesfile_line( 'mes\\spell.mes', 30001 )
	else:
		# Failed saving throw - apply nausea.
		status = STATE_NAUSEA
		print critter, " is in stench, failed saving throw - applying nausea"
		critter.float_mesfile_line( 'mes\\spell.mes', 30002 )
	return status


# Give the critter an effect object with the appropriate effect.
def applyStenchEffect(casterId, spell_id, critter, status):
	stench_obj = getStenchObj(critter)
	if (stench_obj != OBJ_HANDLE_NULL):
		if (status == STATE_NAUSEA):
			# Check current stench effect.
			effectOwnerId = getObjVarNibble(stench_obj, EFFECT_VAR, CID)
			effectStatus = getObjVarNibble(stench_obj, EFFECT_VAR, effectOwnerId)
			if (effectStatus == STATE_SICKNESS):
				# Take ownership of effect since Nausea trumps Sickness.
				print "caster ", casterId, " taking ownership of effect from caster ", effectOwnerId, " on ", critter
				effectVar = getObjVarDWord(stench_obj, EFFECT_VAR)
				stench_obj.destroy()
				stench_obj = createStenchObject(spell_id, critter, status)
				setObjVarDWord(stench_obj, EFFECT_VAR, effectVar)
	else:
		stench_obj = createStenchObject(spell_id, critter, status)
	# Set the status & id into the effect var.
	setObjVarNibble(stench_obj, EFFECT_VAR, CID, casterId)
	setObjVarNibble(stench_obj, EFFECT_VAR, casterId, status)
	return


def removeStenchEffect(casterId, spell_id, critter, status, unNullify):
	stench_obj = getStenchObj(critter)
	if (stench_obj != OBJ_HANDLE_NULL):
		print "removing effect ", stench_obj, " for ", critter, " by caster ", casterId
		setObjVarNibble(stench_obj, EFFECT_VAR, casterId, status)
		# Check current stench effect.
		effectOwnerId = getObjVarNibble(stench_obj, EFFECT_VAR, CID)
		if (casterId == effectOwnerId):
			# Pass ownership of effect to another stench.
			for otherId in xrange(BASECASTERID, 7):
				s = getObjVarNibble(stench_obj, EFFECT_VAR, otherId)
				if (s in (STATE_NAUSEA, STATE_NAUSEA_HANGOVER)):
					break		# Nausea trumps Sickness.
			effectVar = getObjVarDWord(stench_obj, EFFECT_VAR)
			stench_obj.destroy()
			if (s in (STATE_NAUSEA, STATE_NAUSEA_HANGOVER, STATE_SICKNESS)):
				print "caster ", otherId, " taking ownership of effect from caster ", casterId, " on ", critter
				stench_obj = createStenchObject(spell_id, critter, s)
				setObjVarDWord(stench_obj, EFFECT_VAR, effectVar)
				setObjVarNibble(stench_obj, EFFECT_VAR, CID, otherId)
			if (s not in (STATE_NAUSEA, STATE_NAUSEA_HANGOVER) and unNullify):
				reenableWeapons(critter)
	else:
		reenableWeapons(critter)
	return


def createStenchObject(spell_id, critter, status):
	stench_obj = game.obj_create(OBJ_SPELL_STENCH, critter.location)
	stench_obj.item_flag_set(OIF_NO_DROP)
	stench_obj.item_flag_set(OIF_NO_LOOT)
	set_spell_flag(stench_obj, OSF_IS_HEZROU_STENCH)
	if (status == STATE_SICKNESS):
		print "status: ", status, " applying sickness effect to ", stench_obj, " for ", critter
		stench_obj.item_condition_add_with_args('sp-Unholy Blight', spell_id, STENCH_DURATION, 0)	# Doesn't penalise saves - does flag char info.
		stench_obj.item_condition_add_with_args('Saving Throw Resistance Bonus', 0, -2)
		stench_obj.item_condition_add_with_args('Saving Throw Resistance Bonus', 1, -2)
		stench_obj.item_condition_add_with_args('Saving Throw Resistance Bonus', 2, -2)
	elif (status == STATE_NAUSEA):
		print "status: ", status, " applying nausea effect to ", stench_obj, " for ", critter
		# Reduce weapon dmg since not supposed to be able to attack when nauseated.
		nullifyWeapons(critter)
		# Apply simulated nausea effects to stench obj.
		strength = critter.stat_level_get(stat_strength)											# No strength bonus.
		stench_obj.item_condition_add_with_args('Attribute Enhancement Bonus', 0, 11 - strength)
		stench_obj.item_condition_add_with_args('sp-Chaos Hammer', spell_id, STENCH_DURATION, 0)	# Only get move action. (+2 AC, -2 attack & damage rolls, +2 saves [shoddy implementation])
		stench_obj.item_condition_add_with_args('sp-Feeblemind', 0,0,0)								# No casting. (-4 to saves for arcane spellcasters)
		if (critter.stat_level_get(stat_level_sorcerer) > 0 or critter.stat_level_get(stat_level_bard) > 0 or critter.stat_level_get(stat_level_wizard) > 0):
			stench_obj.item_condition_add_with_args('Saving Throw Resistance Bonus', 0, 2)
			stench_obj.item_condition_add_with_args('Saving Throw Resistance Bonus', 1, 2)
			stench_obj.item_condition_add_with_args('Saving Throw Resistance Bonus', 2, 2)	# Counteract feeblemind penalties for arcane casters.
		else:
			stench_obj.item_condition_add_with_args('Saving Throw Resistance Bonus', 0, -2)
			stench_obj.item_condition_add_with_args('Saving Throw Resistance Bonus', 1, -2)
			stench_obj.item_condition_add_with_args('Saving Throw Resistance Bonus', 2, -2)	# Counteract hammer bonuses.

	critter.item_get(stench_obj)
	game.particles("sp-Stinking Cloud Hit", critter)
	return stench_obj


def getStenchObj(critter):
	stench_obj = critter.item_find(OBJ_SPELL_STENCH)
	while (stench_obj != OBJ_HANDLE_NULL and not is_spell_flag_set(stench_obj, OSF_IS_HEZROU_STENCH)):
		stench_obj = critter.item_find(OBJ_SPELL_STENCH)
	return stench_obj


def neutraliseStench(critter, duration):
	print "Neutralising stench on: ", critter
	stench_obj = getStenchObj(critter)
	if (stench_obj != OBJ_HANDLE_NULL):
		stench_obj.destroy()
		reenableWeapons(critter)
	# Add to cured count and set all stench id's to neutralised.
	curedCount = getObjVarNibble(critter, STATE_VAR, COUNTER)
	setObjVarNibble(critter, STATE_VAR, COUNTER, curedCount + 1)
	for casterId in xrange(BASECASTERID, 7):
		setObjVarNibble(critter, STATE_VAR, casterId, STATE_NEUTRALISED)
	# Setup processing for un-neutralisation.
	game.timevent_add(unNeutraliseStench, (critter), 1000 * duration)
	return

def unNeutraliseStench(critter):
	curedCount = getObjVarNibble(critter, STATE_VAR, COUNTER)
	print "Un-neutralising stench, curedCount=", curedCount
	setObjVarNibble(critter, STATE_VAR, COUNTER, curedCount - 1)
	if (curedCount <= 1):
		print "Un-neutralising stench on: ", critter
		for casterId in xrange(BASECASTERID, 7):
			setObjVarNibble(critter, STATE_VAR, casterId, STATE_UNPROCESSED)
	return
	
	
def nullifyWeapons(critter):
	for num in xrange(4000, 5000):
		weapon = critter.item_find_by_proto(num)
		if (weapon != OBJ_HANDLE_NULL and not is_spell_flag_set(weapon, OSF_IS_HEZROU_STENCH)):
			print "Nullifying weapon ", weapon, " for ", critter
			set_spell_flag(weapon, OSF_IS_HEZROU_STENCH)
			weapon.obj_set_int(obj_f_weapon_pad_i_2, weapon.obj_get_int(obj_f_weapon_damage_dice))
			weapon.obj_set_int(obj_f_weapon_damage_dice, 0)
			weapon.item_flag_set(OIF_NO_DROP)
	return

def reenableWeapons(critter):
	for num in xrange(4000, 5000):
		weapon = critter.item_find_by_proto(num)
		if (weapon != OBJ_HANDLE_NULL and is_spell_flag_set(weapon, OSF_IS_HEZROU_STENCH)):
			print "Enabling weapon ", weapon, " for ", critter
			weapon.obj_set_int(obj_f_weapon_damage_dice, weapon.obj_get_int(obj_f_weapon_pad_i_2))
			unset_spell_flag(weapon, OSF_IS_HEZROU_STENCH)
			weapon.item_flag_unset(OIF_NO_DROP)
	return


def endStench(caster, spell_id):
	casterId = getObjVarNibble(caster, STATE_VAR, CID)
	print "Cleaning up and ending stench for ", caster, " id=", casterId
	for critter in game.obj_list_vicinity(caster.location, OLC_CRITTERS):
		if (getObjVarNibble(critter, STATE_VAR, casterId) != STATE_UNPROCESSED):
			removeStenchEffect(casterId, spell_id, critter, STATE_UNPROCESSED, true)
		if (getObjVarNibble(critter, STATE_VAR, casterId) != STATE_NEUTRALISED):
			setObjVarNibble(critter, STATE_VAR, casterId, STATE_UNPROCESSED)
	return

def has_necklace(critter):
	full = critter.item_worn_at(1)
	if full != OBJ_HANDLE_NULL and full.name == 6107:
		return 1
	return 0


#------------------------------------------------------------------------------
# Originally in Co8.py
#------------------------------------------------------------------------------

#################################################################
# Added by Hazelnut                                             #
#################################################################

# Replacement for the D20STD_F_POISON flag for saving throws. The STD define contains
#	the enum index value, 4, which is incorrect as it's checked against the bitmask 8
#	in temple.dll.
D20CO8_F_POISON = 8

# Util functions for getting & setting words, bytes and nibbles in object integers.
# object = reference to the object containing the integer variable.
# var	 = the variable to be used. e.g. obj_f_weapon_pad_i_2
# idx	 = the index of the word (0-1), byte (0-3) or nibble (0-7) to use.
# val	 = the value to be set.

def getObjVarDWord(object, var):
	return object.obj_get_int(var)

def setObjVarDWord(object, var, val):
	object.obj_set_int(var, val)

def getObjVarWord(object, var, idx):
	return getObjVar(object, var, idx, 0xffff, 16)

def setObjVarWord(object, var, idx, val):
	setObjVar(object, var, idx, val, 0xffff, 16)

def getObjVarByte(object, var, idx):
	return getObjVar(object, var, idx, 0xff, 8)

def setObjVarByte(object, var, idx, val):
	setObjVar(object, var, idx, val, 0xff, 8)

def getObjVarNibble(object, var, idx):
	return getObjVar(object, var, idx, 0xf, 4)

def setObjVarNibble(object, var, idx, val):
	setObjVar(object, var, idx, val, 0xf, 4)

def getObjVar(object, var, idx, mask, bits):
	bitMask = mask << (idx * bits)
	val = object.obj_get_int(var) & bitMask
	val = val >> (idx * bits)
	return (val & mask)

def setObjVar(object, var, idx, val, mask, bits):
	#print "obj=", object, " var=", var, " idx=", idx, " val=", val
	bitMask = mask << (idx * bits)
	val = val << (idx * bits)
	oldVal = object.obj_get_int(var) & ~bitMask
	object.obj_set_int(var, oldVal | val)

