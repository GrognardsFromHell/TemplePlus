from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import math

# Divine Spell Power:  Complete Divine, p. 80

divineSpellPowerEnum = 2603

print "Registering Divine Spell Power"

def DivineSpellPowerRadial(attachee, args, evt_obj):
	isAdded = attachee.condition_add_with_args("Divine Spell Power Effect",0,0, 0, 0) # adds the "Divine Spell Power" condition on first radial menu build
	radialAction = tpdp.RadialMenuEntryPythonAction(-1, D20A_PYTHON_ACTION, divineSpellPowerEnum, 0, "TAG_INTERFACE_HELP")
	radialAction.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def OnDivineSpellPowerCheck(attachee, args, evt_obj):
	#Get the current number of turn charges
	TurnCharges = attachee.d20_query("Turn Undead Charges")

	#Check for remaining turn undead attempts
	if (TurnCharges < 1):
		evt_obj.return_val = AEC_OUT_OF_CHARGES
		return 0

	#Check that the character is not a fallen paladin without black guard levels
	if attachee.d20_query(Q_IsFallenPaladin) and (attachee.stat_level_get(stat_level_blackguard) == 0):
		evt_obj.return_val = AEC_INVALID_ACTION
		return 0
		
	#Don't allow a second use in a single round
	if args.get_arg(0): 
		evt_obj.return_val = AEC_INVALID_ACTION
		return 0

	return 1

def OnDivineSpellPowerPerform(attachee, args, evt_obj):
	#Set to active
	args.set_arg(0, 1) 

	#Deduct a turn undead charge
	attachee.d20_send_signal("Deduct Turn Undead Charge")

	#Roll a turn undead check (charisma check) with a + 3 modifier
	cha_bonus = (attachee.stat_level_get(stat_charisma) - 10) / 2
	roll = game.random_range(1,20) + cha_bonus + 3
	
	#Calculate the level bonus
	LevelBonus = int(math.ceil(roll)/3) - 4
	
	#Turn Undead Level Maxes out at + or - 4 levels
	LevelBonus = min(LevelBonus, 4)
	LevelBonus = max(LevelBonus, -4)
	
	#Set the Bonus
	args.set_arg(1, LevelBonus)

	return 0

def DivineSpellPowerBeginRound(attachee, args, evt_obj):
	args.set_arg(0, 0) # always remove at the begining of the round
	args.set_arg(1, 0) # set to zero bonus
	args.set_arg(2, 0) # set spell cast count to zero
	return 0

def DivineSpellPowerCasterLevelBonus(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0): 
		return 0
		
	#If this is the second spell cast with spellpower, don't apply the bonus
	if args.get_arg(2) > 0: 
		return 0
	
	spellPkt = evt_obj.get_spell_packet()
	if spellPkt.is_divine_spell():
		# Prevent invalid caster levels
		casterBonus = args.get_arg(1)
		casterLevel = evt_obj.return_val + casterBonus
		evt_obj.return_val = max(casterLevel, 1)
		
	return 0

def DivineSpellPowerTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0

	# Set the tooltip
	evt_obj.append("Divine Spell Power")

	return 0

def DivineSpellPowerEffectTooltip(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0
	
	#Once a spell has been cast disable the tooltip
	if args.get_arg(2):
		return 0

	# Set the tooltip
	evt_obj.append(tpdp.hash("DIVINE_SPELL_POWER"), -2, " (caster level bonus: " + str(args.get_arg(1)) + ")")
	return 0
	
def DivineSpellPowerCastSpell(attachee, args, evt_obj):
	# not active, do nothing
	if not args.get_arg(0):
		return 0
	
	#Incriment the spell cast with spell power count
	sepllCastCount = args.get_arg(2)
	args.set_arg(2, sepllCastCount + 1)
	
	return 0

#Setup the feat
DivineSpellPowerFeat = PythonModifier("Divine Spell Power Feat", 4) 
DivineSpellPowerFeat.MapToFeat("Divine Spell Power")
DivineSpellPowerFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, DivineSpellPowerRadial, ())

#Setup the effect
DivineSpellPowerEffect = PythonModifier("Divine Spell Power Effect", 4) #Enabled, Bonus, Extra, Extra
DivineSpellPowerEffect.AddHook(ET_OnD20PythonActionCheck, divineSpellPowerEnum, OnDivineSpellPowerCheck, ())
DivineSpellPowerEffect.AddHook(ET_OnD20PythonActionPerform, divineSpellPowerEnum, OnDivineSpellPowerPerform, ())
DivineSpellPowerEffect.AddHook(ET_OnBeginRound, EK_NONE, DivineSpellPowerBeginRound, ())
DivineSpellPowerEffect.AddHook(ET_OnGetTooltip, EK_NONE, DivineSpellPowerTooltip, ())
DivineSpellPowerEffect.AddHook(ET_OnGetCasterLevelMod, EK_NONE, DivineSpellPowerCasterLevelBonus, ())
DivineSpellPowerEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, DivineSpellPowerEffectTooltip, ())
DivineSpellPowerEffect.AddHook(ET_OnD20Signal, EK_S_Spell_Cast, DivineSpellPowerCastSpell, ()) 
