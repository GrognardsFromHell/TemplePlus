from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
print "Registering sp-Magic Circle Outward Fixed"

# args: (0-4)
# 0 - spell_id
# 1 - duration
# 2 - Type Flag (1-Good, 2- Evil, 3- Law, 4- Chaos)
# 3 - aoe id
# 4 - spare

#List of spells protection from evil grants immunity for (should really suppress).
#Includes charm/dominate enchantments (not sleep or confusion) and magic jar (if someone ever adds it).
immunity_list = [
	spell_mass_charm_monster, spell_charm_monster, spell_charm_person, spell_charm_person_or_animal, spell_magic_jar, spell_dominate_animal, 
	spell_dominate_monster, spell_dominate_person, spell_mass_suggestion, spell_suggestion 
]

#Checks if a character's alignment matches the spell type
def CheckAlignment(character, flag):
	
	align = character.stat_level_get(stat_alignment)
	if flag == 1:
		return align & ALIGNMENT_GOOD
	elif flag == 2:
		return align & ALIGNMENT_EVIL
	elif flag == 3:
		return align & ALIGNMENT_LAWFUL
	elif flag == 4:
		return align & ALIGNMENT_CHAOTIC

	return 0
	
#Check if a summoned creature can still attack someone protected
def SummonCanAttack(character, flag):
	
	align = character.stat_level_get(stat_alignment)
	if flag == 1:
		return align & ALIGNMENT_EVIL
	elif flag == 2:
		return align & ALIGNMENT_GOOD
	elif flag == 4:
		return align & ALIGNMENT_CHAOTIC
	elif flag == 3:
		return align & ALIGNMENT_LAWFUL

	return 0

#Gets the text name for the type of spell
def GetCircleSpellType(type):
	SpellName = ""
	if type == 1:
		SpellName = game.get_spell_mesline(284)
	elif type == 2:
		SpellName = game.get_spell_mesline(283)
	elif type == 3:
		SpellName = game.get_spell_mesline(285)
	elif type == 4:
		SpellName = game.get_spell_mesline(282)
	return SpellName
	
#Gets the text name for the type of effect
def GetCircleEffectType(type):
	SpellName = ""
	if type == 1:
		SpellName = game.get_spell_mesline(371)
	elif type == 2:
		SpellName = game.get_spell_mesline(370)
	elif type == 3:
		SpellName = game.get_spell_mesline(372)
	elif type == 4:
		SpellName = game.get_spell_mesline(368)
	return SpellName

def MagicCircleOutwardBegin(attachee, args, evt_obj):
	radius_feet = 10.0
	obj_evt_id = attachee.object_event_append(OLC_CRITTERS, radius_feet)
	args.set_arg(3, obj_evt_id)
	print "Magic Circle Outward: New Object Event ID: " + str(obj_evt_id)
	return 0

def MagicCircleOutwardAoEEntered(attachee, args, evt_obj):
	obj_evt_id = args.get_arg(3)

	if obj_evt_id != evt_obj.evt_id:
		print "Magic Circle Outward Aura Entered: ID mismatch " + str(evt_obj.evt_id) + ", stored was: " + str(obj_evt_id)
		return 0

	print "Magic Circle Outward Aura Entered, event ID: " + str(obj_evt_id)
	tgt = evt_obj.target
	if tgt == OBJ_HANDLE_NULL:
		return 0
	if attachee == OBJ_HANDLE_NULL:
		return 0

	#All get the effect even the character the spell is on
	type = args.get_arg(2)
	spell_id = args.get_arg(0)
	tgt.condition_add_with_args("Magic Circle Outward Aura", obj_evt_id, type, spell_id)
	return 0
	
def MagicCircleOutwardHasSpellActive(attachee, args, evt_obj):
	evt_obj.return_val = 1
	return 0
	
def MagicCircleOutwardKilled(attachee, args, evt_obj):
	args.remove_spell()
	args.remove_spell_mod()
	return 0
	
def MagicCircleOutwardSpellEnd(attachee, args, evt_obj):
	spell_id = args.get_arg(0)
	type = args.get_arg(2)
	if evt_obj.data1 == spell_id:
		if type == 1:
			game.particles( 'sp-Magic Circle against Good-END', attachee)
		elif type == 2:
			game.particles( 'sp-Magic Circle against Evil-END', attachee)
		elif type == 3:
			game.particles( 'sp-Magic Circle against Law-END', attachee)
		elif type == 4:
			game.particles( 'sp-Magic Circle against Chaos-END', attachee)
	return 0
	
def HasMagicCircleQuery(attachee, args, evt_obj):
	type = args.get_arg(2)
	if evt_obj.data1 == type:
		evt_obj.return_val = 1
	return 0
	
def MagicCircleOutwardTooltip(attachee, args, evt_obj):
	type = args.get_arg(2)
	SpellName = GetCircleSpellType(type)
	evt_obj.append(SpellName)
	return 0
	
def MagicCircleOutwardEffectTooltip(attachee, args, evt_obj):
	type = args.get_arg(2)
	SpellName = GetCircleSpellType(type)
	evt_obj.append(tpdp.hash("MAGIC_CIRCLE_OUTWARD_FIXED"), -2, SpellName + "")
	return 0

magicCircleOutward = PythonModifier("sp-Magic Circle Outward Fixed", 5, 0) #Note:  Allows duplicates (for the other versions of the spell)
magicCircleOutward.AddHook(ET_OnConditionAdd, EK_NONE, MagicCircleOutwardBegin, ())
magicCircleOutward.AddHook(ET_OnD20Signal, EK_S_Teleport_Reconnect, MagicCircleOutwardBegin, ())
magicCircleOutward.AddHook(ET_OnObjectEvent, EK_OnEnterAoE, MagicCircleOutwardAoEEntered, ())
magicCircleOutward.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, MagicCircleOutwardHasSpellActive, ())
magicCircleOutward.AddHook(ET_OnD20Signal, EK_S_Killed, MagicCircleOutwardKilled, ())
magicCircleOutward.AddHook(ET_OnD20Signal, EK_S_Spell_End, MagicCircleOutwardSpellEnd, ())
magicCircleOutward.AddHook(ET_OnD20PythonQuery, "Has Magic Circle Spell", HasMagicCircleQuery, ())
magicCircleOutward.AddHook(ET_OnGetTooltip, EK_NONE, MagicCircleOutwardTooltip, ())
magicCircleOutward.AddHook(ET_OnGetEffectTooltip, EK_NONE, MagicCircleOutwardEffectTooltip, ())
magicCircleOutward.AddSpellDispelCheckStandard()
magicCircleOutward.AddSpellTeleportPrepareStandard()
magicCircleOutward.AddSpellTeleportReconnectStandard()
magicCircleOutward.AddSpellCountdownStandardHook()


def MagicCircleOutwardEffAoEExited(attachee, args, evt_obj):
	obj_evt_id = args.get_arg(0)
	if obj_evt_id != evt_obj.evt_id:
		print "Magic Circle Outward Aura: ID mismatch " + str(evt_obj.evt_id) + ", stored was: " + str(obj_evt_id)
		return 0
	print "Magic Circle Outward Aura (ID " + str(obj_evt_id) +") Exited, critter: " + attachee.description + " "
	args.condition_remove()
	return 0

def MagicCircleOutwardEffTooltip(attachee, args, evt_obj):
	type = args.get_arg(1)
	SpellName = GetCircleEffectType(type)
	evt_obj.append(SpellName)
	return 0
	
def MagicCircleOutwardEffEffectTooltip(attachee, args, evt_obj):
	type = args.get_arg(1)
	SpellName = GetCircleEffectType(type)
	evt_obj.append(tpdp.hash("MAGIC_CIRCLE_OUTWARD_FIXED_EFFECT"), -2, SpellName + "")
	return 0

def MagicCircleOutwardEffRemove(attachee, args, evt_obj):
	print "Removing Magic Circle Effect Condition " + attachee.description
	args.condition_remove()
	return 0
	
def MagicCircleOutwardEffACBonus(attachee, args, evt_obj):
	attacker = evt_obj.attack_packet.attacker
	if attacker == OBJ_HANDLE_NULL:
		return 0
	
	type = args.get_arg(1)
	addBonus = CheckAlignment(attacker, type)
	
	if addBonus:
		evt_obj.bonus_list.add(2, 11, 207) #Deflection bonus
	return 0

def MagicCircleOutwardEffSavingThrow(attachee, args, evt_obj):
	caster = evt_obj.obj
	type = args.get_arg(1)
	
	addBonus = CheckAlignment(caster, type)
	if addBonus:
		evt_obj.bonus_list.add(2, 15, 207) #Resistance bonus
	return 0
	
def MagicCircleOutwardEffDamageResistance(attachee, args, evt_obj):
	attacker = evt_obj.attack_packet.attacker
	
	if attacker == OBJ_HANDLE_NULL:
		return 0
	
	if attacker.d20_query_has_condition("sp-Summoned"):
		type = args.get_arg(1)
		canAttack = SummonCanAttack(attacker, type)
		if not canAttack:
			wpn = evt_obj.attack_packet.get_weapon_used()
			if wpn != OBJ_HANDLE_NULL:
				return 0

			spell_id = args.get_arg(2)
			spell_packet = tpdp.SpellPacket(spell_id)
			resisted = spell_packet.check_spell_resistance_force(attacker)
			if resisted:
				return 0
			
			evt_obj.damage_packet.add_mod_factor(0.0, D20DT_UNSPECIFIED, 104) #Do no damage at all
	return 0
	
def MagicCircleOutwardEffSpellImmunity(attachee, args, evt_obj):
	sp_pkt = evt_obj.spell_packet
	spell_enum = sp_pkt.spell_enum
	if (spell_enum == 0):
		return 0
		
	# Providing immunity (the effect really should just be suppressed while inside the circle)
	if spell_enum in immunity_list:
		evt_obj.return_val = 1
	return 0

def MagicCircleOutwardEffPreAdd(attachee, args, evt_obj):
	val = evt_obj.is_modifier("Magic Circle Outward Aura") #Is it a duplicate?
	if val:
		type = args.get_arg(1)
		if type == evt_obj.arg2:  #Reject if it is the same type of protection effect
			evt_obj.return_val = 0
	return 0

#  Note:  Condition allows duplicates for the other versions of the spell
magicCircleOutwardEffect = PythonModifier("Magic Circle Outward Aura", 4, 0) #id, alignment flag (1-Good, 2- Evil, 3- Law, 4- Chaos), spell id, spare 
magicCircleOutwardEffect.AddHook(ET_OnObjectEvent, EK_OnLeaveAoE, MagicCircleOutwardEffAoEExited, ())
magicCircleOutwardEffect.AddHook(ET_OnNewDay, EK_NEWDAY_REST, MagicCircleOutwardEffRemove, ())
magicCircleOutwardEffect.AddHook(ET_OnGetTooltip, EK_NONE, MagicCircleOutwardEffTooltip, ())
magicCircleOutwardEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, MagicCircleOutwardEffEffectTooltip, ())
magicCircleOutwardEffect.AddHook(ET_OnD20Signal, EK_S_Teleport_Prepare, MagicCircleOutwardEffRemove, ())
magicCircleOutwardEffect.AddHook(ET_OnGetAC, EK_NONE, MagicCircleOutwardEffACBonus, ())
magicCircleOutwardEffect.AddHook(ET_OnSaveThrowLevel, EK_NONE, MagicCircleOutwardEffSavingThrow, () )
magicCircleOutwardEffect.AddHook(ET_OnTakingDamage2, EK_NONE, MagicCircleOutwardEffDamageResistance, ())
magicCircleOutwardEffect.AddHook(ET_OnSpellImmunityCheck, EK_NONE, MagicCircleOutwardEffSpellImmunity, ())
magicCircleOutwardEffect.AddHook(ET_OnConditionAddPre, EK_NONE, MagicCircleOutwardEffPreAdd, ())
