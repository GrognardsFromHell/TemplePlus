from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import math
from spell_utils import skillCheck

# Captivating Melody:  Complete Mage, p. 40

captivatingMelodyEnum = 3000

print "Registering Captivating Melody"

def CaptivatingMelodyRadial(attachee, args, evt_obj):
	isAdded = attachee.condition_add_with_args("Captivating Melody Effect",0,0, 0, 0) # adds the "Captivating Melody" condition on first radial menu build
	radialAction = tpdp.RadialMenuEntryPythonAction(-1, D20A_PYTHON_ACTION, captivatingMelodyEnum, 0, "TAG_INTERFACE_HELP")
	radialAction.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def OnCaptivatingMelodyCheck(attachee, args, evt_obj):
	#Get the current number of charges
	MusicCharges = attachee.d20_query("Current Bardic Music")

	#Check for remaining bardic music uses
	if (MusicCharges < 1):
		evt_obj.return_val = AEC_OUT_OF_CHARGES
		return 0
		
	#Don't allow a second use in a single round
	if args.get_arg(0): 
		evt_obj.return_val = AEC_INVALID_ACTION
		return 0

	return 1

def CaptivatingMelodyPerform(attachee, args, evt_obj):
	#Set to active
	args.set_arg(0, 1) 

	#Deduct a turn undead charge
	attachee.d20_send_signal("Deduct Bardic Music Charge")

	return 0

def CaptivatingMelodyBeginRound(attachee, args, evt_obj):
	args.set_arg(0, 0) # always remove at the begining of the round
	args.set_arg(1, 0) # no spells cast this round
	return 0

def CaptivatingMelodyDCBonus(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0): 
		return 0
		
	#If this is not the first spell cast, don't apply the bonus
	if args.get_arg(2) > 0: 
		return 0
	
	#Must be the appropraite type of spell
	spell_enum = evt_obj.spell_packet.spell_enum
	if (spell_enum == 0):
		return 0
	spell_entry = tpdp.SpellEntry(spell_enum)
	
	if spell_entry.spell_school_enum != Enchantment and spell_entry.spell_school_enum != Illusion:
		return 0
		
	if evt_obj.spell_packet.get_spell_casting_class() != stat_level_bard:
		return 0
	
	#Make a perform check
	performDC = evt_obj.spell_packet.spell_known_slot_level + 15
	result = skillCheck(attachee, skill_perform, performDC)
	if result == False:
		return 0
		
	#Finally add the bonus
	evt_obj.bonus_list.add(2, 0, "Captivating Melody")
		
	return 0

def CaptivatingMelodyTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0

	# Set the tooltip
	evt_obj.append("Captivating Melody")

	return 0

def CaptivatingMelodyEffectTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0
	
	#Once a spell has been cast disable the tooltip
	if args.get_arg(2):
		return 0

	# Set the tooltip
	evt_obj.append(tpdp.hash("CAPTIVATING_MELODY"), -2, "")
	return 0
	
def CaptivatingMelodyCastSpell(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0
	
	#Incriment the spell cast with spell power count
	sepllCastCount = args.get_arg(1)
	args.set_arg(1, sepllCastCount + 1)
	return 0

#Setup the feat
CaptivatingMelodyFeat = PythonModifier("Captivating Melody Feat", 4) 
CaptivatingMelodyFeat.MapToFeat("Captivating Melody")
CaptivatingMelodyFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, CaptivatingMelodyRadial, ())

#Setup the effect
CaptivatingMelodyEffect = PythonModifier("Captivating Melody Effect", 4) #Enabled, Spell Cast, Extra, Extra
CaptivatingMelodyEffect.AddHook(ET_OnD20PythonActionCheck, captivatingMelodyEnum, OnCaptivatingMelodyCheck, ())
CaptivatingMelodyEffect.AddHook(ET_OnD20PythonActionPerform, captivatingMelodyEnum, CaptivatingMelodyPerform, ())
CaptivatingMelodyEffect.AddHook(ET_OnBeginRound, EK_NONE, CaptivatingMelodyBeginRound, ())
CaptivatingMelodyEffect.AddHook(ET_OnGetTooltip, EK_NONE, CaptivatingMelodyTooltip, ())
CaptivatingMelodyEffect.AddHook(ET_OnTargetSpellDCBonus, EK_NONE, CaptivatingMelodyDCBonus, ())
CaptivatingMelodyEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, CaptivatingMelodyEffectTooltip, ())
CaptivatingMelodyEffect.AddHook(ET_OnD20Signal, EK_S_Spell_Cast, CaptivatingMelodyCastSpell, ()) 
