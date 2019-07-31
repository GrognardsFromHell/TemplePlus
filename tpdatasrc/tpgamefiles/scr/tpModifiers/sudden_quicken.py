from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Sudden Quicken:  Complete Arcane, p. 83

print "Registering Sudden Quicken"

def SuddenQuickenRadial(attachee, args, evt_obj):
	#Add a checkbox to turn on and off sudden quicken if there is a carge available, otherwise just don't show it
	if args.get_arg(0):
		checkboxSuddenquicken = tpdp.RadialMenuEntryToggle("Sudden Quicken", "TAG_INTERFACE_HELP")
		checkboxSuddenquicken.link_to_args(args, 1)
		checkboxSuddenquicken.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def SuddenQuickenNewDay(attachee, args, evt_obj):
	#One charge per day
	args.set_arg(0, 1)
	
	#Set the checkbox to off at the begining of the day
	args.set_arg(1, 0)
	
	return 0

def SuddenQuickenMetamagicUpdate(attachee, args, evt_obj):
	#Check for a charge
	charges = args.get_arg(0)
	if charges < 1:
		return 0
	
	#Check if sudden quicken is turned on
	if not args.get_arg(1):
		return 0
	
	#Get the metamagic info
	metaMagicData = evt_obj.meta_magic
	
	#Don't quicken more than once
	if metaMagicData.get_quicken() < 1:
		metaMagicData.set_quicken(1)
	
	return 0
	
	
def SuddenQuickenDeductCharge(attachee, args, evt_obj):
	#Check for a charge and the enable flag
	charges = args.get_arg(0)
	if charges < 1 or not args.get_arg(1):
		return 0
		
	#Decriment the charges
	charges = charges - 1
	
	#Prevent the issue where the charges start with a large value
	if charges > 1000:
		charges = 0
	
	args.set_arg(0, charges)

	return 0

#Setup the feat
tpdp.register_metamagic_feat("Sudden Quicken")
suddenQuickenFeat = PythonModifier("Sudden Quicken Feat", 4) #Charges, Toggeled On, Spare, Spare
suddenQuickenFeat.MapToFeat("Sudden Quicken")
suddenQuickenFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, SuddenQuickenRadial, ())
suddenQuickenFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, SuddenQuickenNewDay, ())
suddenQuickenFeat.AddHook(ET_OnMetaMagicMod, EK_NONE, SuddenQuickenMetamagicUpdate, ())
suddenQuickenFeat.AddHook(ET_OnD20PythonSignal, "Sudden Metamagic Deduct Charge", SuddenQuickenDeductCharge, ())
