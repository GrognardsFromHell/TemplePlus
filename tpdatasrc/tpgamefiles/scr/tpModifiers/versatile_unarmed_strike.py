from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Versatile Unarmed Strike:  Player's Handbook II, p. 85

VersatileUnarmedStrikeBludgeoningEnum = 2800
VersatileUnarmedStrikePiercingEnum = 2801
VersatileUnarmedStrikeSlashingEnum = 2802

print "Registering Versatile Unarmed Strike"

def GetDamageTypeFromEnum(enum):
	#Determine the intended damage type
	if enum == VersatileUnarmedStrikePiercingEnum:
		return D20DT_PIERCING
	elif enum == VersatileUnarmedStrikeSlashingEnum:
		return D20DT_SLASHING
	return D20DT_BLUDGEONING


def VersatileUnarmedStrikeRadial(attachee, args, evt_obj):

	isAdded = attachee.condition_add_with_args("Versatile Unarmed Strike",0,0) # adds the "Wolverine Rage" condition on first radial menu build
	
	radial_parent = tpdp.RadialMenuEntryParent("Versatile Unarmed Strike")
	VersatileUnarmedStrikeId = radial_parent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	
	#0 - Bludgeoning, 1 - Piercing, 2 - Slashing
	radialAction = tpdp.RadialMenuEntryPythonAction("Bludgeoning", D20A_PYTHON_ACTION, VersatileUnarmedStrikeBludgeoningEnum, 0, "TAG_INTERFACE_HELP")
	radialAction.add_as_child(attachee, VersatileUnarmedStrikeId)
	radialAction = tpdp.RadialMenuEntryPythonAction("Piercing", D20A_PYTHON_ACTION, VersatileUnarmedStrikePiercingEnum, 1, "TAG_INTERFACE_HELP")
	radialAction.add_as_child(attachee, VersatileUnarmedStrikeId)
	radialAction = tpdp.RadialMenuEntryPythonAction("Slashing", D20A_PYTHON_ACTION, VersatileUnarmedStrikeSlashingEnum, 2, "TAG_INTERFACE_HELP")
	radialAction.add_as_child(attachee, VersatileUnarmedStrikeId)
	
	return 0

def OnVersatileUnarmedStrikeEffectCheck(attachee, args, evt_obj):

	damageType = GetDamageTypeFromEnum(evt_obj.d20a.data1)
	
	#Don't allow if it is a change to the same type as is already set
	if args.get_arg(0) == damageType:
		evt_obj.return_val = AEC_INVALID_ACTION
		return 0
		
	#Don't allow if it has already been used this round
	if args.get_arg(1):
		evt_obj.return_val = AEC_INVALID_ACTION
		return 0	

	return 1

def OnVersatileUnarmedStrikeEffectPerform(attachee, args, evt_obj):
	damageType = GetDamageTypeFromEnum(evt_obj.d20a.data1)
		
	#Set to the appropriate damage type
	args.set_arg(0, damageType)
	
	#Set the used this round flag
	args.set_arg(1, 1)
	
	return 0

	
def VersatileUnarmedStrikeEffectBeginRound(attachee, args, evt_obj):
	# Clear the used this round flag
	args.set_arg(1, 0)
	return 0

def VersatileUnarmedStrikeEffectTooltip(attachee, args, evt_obj):
	# 0 is Bludgeoning since this is the default for unarmed combat, don't display
	if not args.get_arg(0):
		return 0
		
	if args.get_arg(0) == 1:
		damageType = "Piercing"
	else:
		damageType = "Slashing"
		
	evt_obj.append("Versatile Unarmed Strike - " + damageType)

	return 0

def VersatileUnarmedStrikeEffectTooltipEffect(attachee, args, evt_obj):
	# 0 is Bludgeoning since this is the default, don't display
	if not args.get_arg(0):
		return 0
		
	if args.get_arg(0) == 1:
		damageType = "Piercing"
	else:
		damageType = "Slashing"

	# Set the tooltip
	evt_obj.append(tpdp.hash("VERSATILE_UNARMED_STRIKE"), -2, " - " + damageType)
	return 0
	
#Responds to the unarmed damage type query
def VersatileUnarmedStrikeDamageType(attachee, args, evt_obj):
	evt_obj.return_val = args.get_arg(0)
	return 0

#Setup the feat
VersatileUnarmedStrikeFeat = PythonModifier("Versatile Unarmed Strike Feat", 2) # spare, spare
VersatileUnarmedStrikeFeat.MapToFeat("Versatile Unarmed Strike")
VersatileUnarmedStrikeFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, VersatileUnarmedStrikeRadial, ())

#Setup the effect
VersatileUnarmedStrikeEffect = PythonModifier("Versatile Unarmed Strike", 4) #enabled, swapped this round, spare, spare
VersatileUnarmedStrikeEffect.AddHook(ET_OnBeginRound, EK_NONE, VersatileUnarmedStrikeEffectBeginRound, ())
VersatileUnarmedStrikeEffect.AddHook(ET_OnD20PythonActionCheck, VersatileUnarmedStrikeBludgeoningEnum, OnVersatileUnarmedStrikeEffectCheck, ())
VersatileUnarmedStrikeEffect.AddHook(ET_OnD20PythonActionPerform, VersatileUnarmedStrikeBludgeoningEnum, OnVersatileUnarmedStrikeEffectPerform, ())
VersatileUnarmedStrikeEffect.AddHook(ET_OnD20PythonActionCheck, VersatileUnarmedStrikePiercingEnum, OnVersatileUnarmedStrikeEffectCheck, ())
VersatileUnarmedStrikeEffect.AddHook(ET_OnD20PythonActionPerform, VersatileUnarmedStrikePiercingEnum, OnVersatileUnarmedStrikeEffectPerform, ())
VersatileUnarmedStrikeEffect.AddHook(ET_OnD20PythonActionCheck, VersatileUnarmedStrikeSlashingEnum, OnVersatileUnarmedStrikeEffectCheck, ())
VersatileUnarmedStrikeEffect.AddHook(ET_OnD20PythonActionPerform, VersatileUnarmedStrikeSlashingEnum, OnVersatileUnarmedStrikeEffectPerform, ())
VersatileUnarmedStrikeEffect.AddHook(ET_OnGetTooltip, EK_NONE, VersatileUnarmedStrikeEffectTooltip, ())
VersatileUnarmedStrikeEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, VersatileUnarmedStrikeEffectTooltipEffect, ())
VersatileUnarmedStrikeEffect.AddHook(ET_OnD20PythonQuery, "Unarmed Damage Type", VersatileUnarmedStrikeDamageType, ())