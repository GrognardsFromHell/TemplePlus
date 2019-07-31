from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Sudden Enlarge:  Miniatures Handbook, p. 28

print "Registering Sudden Enlarge"

def SuddenEnlargeRadial(attachee, args, evt_obj):
	#Add a checkbox to turn on and off sudden Enlarge if there is a carge available, otherwise just don't show it
	if args.get_arg(0):
		checkboxSuddenEnlarge = tpdp.RadialMenuEntryToggle("Sudden Enlarge", "TAG_INTERFACE_HELP")
		checkboxSuddenEnlarge.link_to_args(args, 1)
		checkboxSuddenEnlarge.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def SuddenEnlargeNewDay(attachee, args, evt_obj):
	#One charge per day
	args.set_arg(0, 1)
	
	#Set the checkbox to off at the begining of the day
	args.set_arg(1, 0)
	
	return 0

def SuddenEnlargeMetamagicUpdate(attachee, args, evt_obj):
	
	#Check for a charge
	charges = args.get_arg(0)
	if charges < 1:
		return 0
	
	#Check if sudden enlarge is turned on
	if not args.get_arg(1):
		return 0
	
	#Get the metamagic info
	metaMagicData = evt_obj.meta_magic
	
	#Don't enlarge more than once
	if metaMagicData.get_enlarge_count() < 1:
		metaMagicData.set_enlarge_count(1)
	
	return 0
	
def SuddenEnlargeDeductCharge(attachee, args, evt_obj):
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
tpdp.register_metamagic_feat("Sudden Enlarge")
suddenEnlargeFeat = PythonModifier("Sudden Enlarge Feat", 4) #Charges, Toggeled On, Spare, Spare
suddenEnlargeFeat.MapToFeat("Sudden Enlarge")
suddenEnlargeFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, SuddenEnlargeRadial, ())
suddenEnlargeFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, SuddenEnlargeNewDay, ())
suddenEnlargeFeat.AddHook(ET_OnMetaMagicMod, EK_NONE, SuddenEnlargeMetamagicUpdate, ())
suddenEnlargeFeat.AddHook(ET_OnD20PythonSignal, "Sudden Metamagic Deduct Charge", SuddenEnlargeDeductCharge, ())
