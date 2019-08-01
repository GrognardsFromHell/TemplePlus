from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Sudden Still:  Complete Arcane, p. 83

print "Registering Sudden Still"

def SuddenStillRadial(attachee, args, evt_obj):
	#Add a checkbox to turn on and off sudden Still if there is a carge available, otherwise just don't show it
	if args.get_arg(0):
		checkboxSuddenStill = tpdp.RadialMenuEntryToggle("Sudden Still", "TAG_INTERFACE_HELP")
		checkboxSuddenStill.link_to_args(args, 1)
		checkboxSuddenStill.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def SuddenStillNewDay(attachee, args, evt_obj):
	#One charge per day
	args.set_arg(0, 1)
	
	#Set the checkbox to off at the begining of the day
	args.set_arg(1, 0)
	
	return 0

def SuddenStillMetamagicUpdate(attachee, args, evt_obj):
	
	#Check for a charge
	charges = args.get_arg(0)
	if charges < 1:
		return 0
	
	#Check if sudden still is turned on
	if not args.get_arg(1):
		return 0
	
	#Get the metamagic info
	metaMagicData = evt_obj.meta_magic
	
	#Don't apply if it already has still
	if not metaMagicData.get_still():
		metaMagicData.set_still(true)
	
	return 0

def SuddenStillDeductCharge(attachee, args, evt_obj):
	#Check for a charge and the enable flag
	charges = args.get_arg(0)
	if charges < 1 or not args.get_arg(1):	
		return 0
		
	#Decriment the charges
	charges = charges - 1
	args.set_arg(0, charges)

	return 0

#Setup the feat
tpdp.register_metamagic_feat("Sudden Still")
suddenStillFeat = PythonModifier("Sudden Still Feat", 4) #Charges, Toggeled On, Spare, Spare
suddenStillFeat.MapToFeat("Sudden Still")
suddenStillFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, SuddenStillRadial, ())
suddenStillFeat.AddHook(ET_OnConditionAdd, EK_NONE, SuddenStillNewDay, ())
suddenStillFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, SuddenStillNewDay, ())
suddenStillFeat.AddHook(ET_OnMetaMagicMod, EK_NONE, SuddenStillMetamagicUpdate, ())
suddenStillFeat.AddHook(ET_OnD20PythonSignal, "Sudden Metamagic Deduct Charge", SuddenStillDeductCharge, ())