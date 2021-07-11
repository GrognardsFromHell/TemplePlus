from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import countAfterConcentration
print "Registering sp-Wall of Fire"

# args: (0-7)
# 0 - spell_id
# 1 - duration
# 2 - event ID
# 3 - angle (milli radians)
# 4 - length (milli feet)

def WallOfFireOnAdd(attachee, args, evt_obj):
	spell_id = args.get_arg(0)
	wall_angle_mrad = args.get_arg(3)
	wall_length_mft = args.get_arg(4)
	wall_angle_rad = wall_angle_mrad / 1000.0
	wall_length_ft = wall_length_mft / 1000.0
	evt_id = attachee.object_event_append_wall(OLC_CRITTERS, wall_length_ft, wall_angle_rad)
	args.set_arg(2, evt_id) # store the event ID
	
	# Update the spell packet
	spell_packet = tpdp.SpellPacket(spell_id)
	
	spell_obj = attachee
	#x,y = location_to_axis(spell_obj.location)
	#print "spell_obj loc x,y: " + str(x) + " " + str(y)
	#print "spell_obj loc off_x: " + str(spell_obj.off_x)
	#print "spell_obj loc off_y: " + str(spell_obj.off_y)
	spell_partsys_id = game.particles('sp-Wall of Fire3', spell_obj)
	spell_packet.add_spell_object(spell_obj, spell_partsys_id) # store the spell obj and the particle sys
	
	origin_loc = attachee.location_full
	
	N_sections = int(round(wall_length_ft / 5.0))
	#print "Wall length(ft): " + str(wall_length_ft) + "  sections: " + str(N_sections)
	for p in range(1, N_sections):
		spell_obj_loc = origin_loc.get_offset_loc(wall_angle_rad, p*5.0)
		spell_obj = game.obj_create(OBJECT_SPELL_GENERIC, spell_obj_loc.get_location(), spell_obj_loc.off_x, spell_obj_loc.off_y)
		spell_obj.move(spell_obj_loc.get_location(), spell_obj_loc.off_x, spell_obj_loc.off_y)
		spell_obj.turn_towards(attachee)
		spell_obj.rotation += 3.1415
		x,y = location_to_axis(spell_obj.location)
		#print "spell_obj loc x,y: " + str(x) + " " + str(y)
		#print "spell_obj loc off_x: " + str(spell_obj.off_x)
		#print "spell_obj loc off_y: " + str(spell_obj.off_y)
		spell_partsys_id = game.particles('sp-Wall of Fire3', spell_obj)
		spell_packet.add_spell_object( spell_obj, spell_partsys_id) # store the spell obj and the particle sys
	
	spell_packet.update_registry()
	
	spell_packet.caster.condition_add_with_args('sp-Concentrating', spell_id)
	return 0


def OnWallAoEEntered(attachee, args, evt_obj):
	#print "Wall of Fire entered event"
	obj_evt_id = args.get_arg(2)
	spell_id = args.get_arg(0)
	spell_packet = tpdp.SpellPacket(spell_id)
	duration = args.get_arg(1)
	caster = spell_packet.caster
	if obj_evt_id != evt_obj.evt_id:
		#print "Wall of Fire Entered: ID mismatch " + str(evt_obj.evt_id) + ", stored was: " + str(obj_evt_id)
		return 0
	
	#print "Wall of Fire Entered, event ID: " + str(obj_evt_id)
	tgt = evt_obj.target
	if tgt == OBJ_HANDLE_NULL or attachee == OBJ_HANDLE_NULL or tgt == attachee:
		return 0
	
	#print str(tgt) + " hit by wall of fire"
	spell_packet.trigger_aoe_hit()
	
	if spell_packet.check_spell_resistance(tgt):
		return 0
	
	# apply sp-Wall of Fire hit condition (applies damage on beginning of round)
	partsys_id = game.particles( "sp-Wall of Fire-hit", tgt )
	if spell_packet.add_target(tgt, partsys_id):
		tgt.condition_add_with_args('sp-Wall of Fire hit', spell_id, duration, obj_evt_id)
	return 0


def OnCombatEnd(attachee, args, evt_obj):
	#print "Combat End"
	spellId = args.get_arg(0)
	spell_packet = tpdp.SpellPacket(spellId)
	if spell_packet.spell_enum == 0:
		return
	if spell_packet.caster != OBJ_HANDLE_NULL:
		spell_packet.float_spell_line(spell_packet.caster, 20000, tf_white)
	args.remove_spell()
	args.remove_spell_mod()
	return 0

wallOfFire = PythonModifier("sp-Wall of Fire", 8)
wallOfFire.AddHook(ET_OnConditionAdd, EK_NONE, WallOfFireOnAdd, ())
wallOfFire.AddHook(ET_OnObjectEvent, EK_OnEnterAoE, OnWallAoEEntered, ())
wallOfFire.AddHook(ET_OnD20Signal, EK_S_Combat_End, OnCombatEnd, ())
wallOfFire.AddHook(ET_OnBeginRound, EK_NONE, countAfterConcentration, ())
wallOfFire.AddAoESpellEndStandardHook()

#wallOfFire.AddSpellDismissStandardHook() # oops, Wall of Fire doesn't have Dismiss (but it does have COncentration...)


##################################################
# sp-Wall of fire hit
# does damage at the beginning of round
##################################################

def EndSpellMod(attachee, args, evt_obj):
	spell_id = args.get_arg(0)
	if evt_obj.data1 == spell_id:
		print "Ending mod for spell ID: " + str(spell_id)
		args.remove_spell_mod() # does a .condition_remove() with some safety checks
	return 0


def WallOfFireBeginRound(attachee, args, evt_obj):
	#print "Wall of Fire begin round"
	tgt = attachee
	spell_id = args.get_arg(0)
	spell_packet = tpdp.SpellPacket(spell_id)
	caster = spell_packet.caster
	damage_dice = dice_new( '2d4' )
	undead_dice = dice_new('4d4')
	
	if tgt.is_category_type( mc_type_undead ):
		tgt.spell_damage(caster, D20DT_FIRE, undead_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell_id)
	else:
		tgt.spell_damage(caster, D20DT_FIRE, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell_id)
	return 0

def WallOfFireHitDamage(attachee, args, evt_obj):
	#print "Wall of Fire hit damage"
	tgt = attachee
	spell_id = args.get_arg(0)
	spell_packet = tpdp.SpellPacket(spell_id)
	caster = spell_packet.caster
	
	damage_dice = dice_new( '2d6' )
	damage_dice.bonus = min( 1 * spell_packet.caster_level, 20 )
	undead_dice = dice_new('4d6')
	undead_dice.bonus = min( 2 * spell_packet.caster_level, 40 )
	
	if tgt.is_category_type( mc_type_undead ):
		tgt.spell_damage(caster, D20DT_FIRE, undead_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell_id)
	else:
		tgt.spell_damage(caster, D20DT_FIRE, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell_id)
	return 0

def OnWallAoEExit(attachee, args, evt_obj):
	evt_id = args.get_arg(2)
	if evt_id != evt_obj.evt_id:
		return 0
	print "Removing sp-Wall of fire hit on " + str(attachee)
	spell_id = args.get_arg(0)
	spell_packet = tpdp.SpellPacket(spell_id)
	spell_packet.end_target_particles(attachee)
	spell_packet.remove_target(attachee)
	args.remove_spell_mod()
	return 0

wallOfFireHit = PythonModifier("sp-Wall of Fire hit", 8)
wallOfFireHit.AddHook(ET_OnBeginRound, EK_NONE, WallOfFireBeginRound, ())
wallOfFireHit.AddHook(ET_OnConditionAdd, EK_NONE, WallOfFireHitDamage, ())
wallOfFireHit.AddHook(ET_OnD20Signal, EK_S_Spell_End, EndSpellMod, ())
wallOfFireHit.AddHook(ET_OnD20Signal, EK_S_Killed, EndSpellMod, ())
wallOfFireHit.AddHook(ET_OnObjectEvent, EK_OnLeaveAoE, OnWallAoEExit, ())
