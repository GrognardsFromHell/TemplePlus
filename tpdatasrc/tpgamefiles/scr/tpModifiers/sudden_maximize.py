from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Sudden Maximize:  Complete Arcane, p. 83

print "Registering Sudden Maximize"

def SuddenMaximizeRadial(attachee, args, evt_obj):
	#Add a checkbox to turn on and off sudden Maximize if there is a carge available, otherwise just don't show it
	if args.get_arg(0):
		checkboxSuddenMaximize = tpdp.RadialMenuEntryToggle("Sudden Maximize", "TAG_INTERFACE_HELP")
		checkboxSuddenMaximize.link_to_args(args, 1)
		checkboxSuddenMaximize.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def SuddenMaximizeNewDay(attachee, args, evt_obj):
	#One charge per day
	args.set_arg(0, 1)
	
	#Set the checkbox to off at the begining of the day
	args.set_arg(1, 0)
	
	return 0

def SuddenMaximizeMetamagicUpdate(attachee, args, evt_obj):
	#Check for a charge
	charges = args.get_arg(0)
	if charges < 1:
		return 0
	
	#Check if sudden Maximize is turned on
	if not args.get_arg(1):
		return 0
	
	#Get the metamagic info
	metaMagicData = evt_obj.meta_magic
	
	#Don't Maximize more than once
	if metaMagicData.get_maximize() < 1:
		metaMagicData.set_maximize(1)
	
	return 0
	
	
def SuddenMaximizeDeductCharge(attachee, args, evt_obj):
	#Check for a charge and the enable flag
	charges = args.get_arg(0)
	if charges < 1 or not args.get_arg(1):
		return 0
		
	#Decriment the charges
	charges = charges - 1
	args.set_arg(0, charges)

	return 0

#Setup the feat
tpdp.register_metamagic_feat("Sudden Maximize")
suddenMaximizeFeat = PythonModifier("Sudden Maximize Feat", 4) #Charges, Toggeled On, Spare, Spare
suddenMaximizeFeat.MapToFeat("Sudden Maximize")
suddenMaximizeFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, SuddenMaximizeRadial, ())
suddenMaximizeFeat.AddHook(ET_OnConditionAdd, EK_NONE, SuddenMaximizeNewDay, ())
suddenMaximizeFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, SuddenMaximizeNewDay, ())
suddenMaximizeFeat.AddHook(ET_OnMetaMagicMod, EK_NONE, SuddenMaximizeMetamagicUpdate, ())
suddenMaximizeFeat.AddHook(ET_OnD20PythonSignal, "Sudden Metamagic Deduct Charge", SuddenMaximizeDeductCharge, ())
