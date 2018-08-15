from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Sudden Empower:  Complete Arcane, p. 83

print "Registering Sudden Empower"

def SuddenEmpowerRadial(attachee, args, evt_obj):
	#Add a checkbox to turn on and off sudden Empower if there is a carge available, otherwise just don't show it
	if args.get_arg(0):
		checkboxSuddenEmpower = tpdp.RadialMenuEntryToggle("Sudden Empower", "TAG_INTERFACE_HELP")
		checkboxSuddenEmpower.link_to_args(args, 1)
		checkboxSuddenEmpower.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def SuddenEmpowerNewDay(attachee, args, evt_obj):
	#One charge per day
	args.set_arg(0, 1)
	
	#Set the checkbox to off at the begining of the day
	args.set_arg(1, 0)
	
	return 0

def SuddenEmpowerMetamagicUpdate(attachee, args, evt_obj):
	#Check for a charge
	charges = args.get_arg(0)
	if charges < 1:
		return 0
	
	#Check if sudden Empower is turned on
	if not args.get_arg(1):
		return 0
	
	#Get the metamagic info
	metaMagicData = evt_obj.meta_magic
	
	#Don't Empower more than once
	if metaMagicData.get_empower_count() < 1:
		metaMagicData.set_empower_count(1)
	
	return 0
	
	
def SuddenEmpowerDeductCharge(attachee, args, evt_obj):
	#Check for a charge and the enable flag
	charges = args.get_arg(0)
	if charges < 1 or not args.get_arg(1):
		return 0
		
	#Decriment the charges
	charges = charges - 1
	args.set_arg(0, charges)

	return 0

#Setup the feat
tpdp.register_metamagic_feat("Sudden Empower")
suddenEmpowerFeat = PythonModifier("Sudden Empower Feat", 4) #Charges, Toggeled On, Spare, Spare
suddenEmpowerFeat.MapToFeat("Sudden Empower")
suddenEmpowerFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, SuddenEmpowerRadial, ())
suddenEmpowerFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, SuddenEmpowerNewDay, ())
suddenEmpowerFeat.AddHook(ET_OnMetaMagicMod, EK_NONE, SuddenEmpowerMetamagicUpdate, ())
suddenEmpowerFeat.AddHook(ET_OnD20PythonSignal, "Sudden Metamagic Deduct Charge", SuddenEmpowerDeductCharge, ())
