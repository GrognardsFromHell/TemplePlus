from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Sudden Extend:  Complete Arcane, p. 83

print "Registering Sudden Extend"

def SuddenExtendRadial(attachee, args, evt_obj):
	#Add a checkbox to turn on and off sudden extend if there is a carge available, otherwise just don't show it
	if args.get_arg(0):
		checkboxSuddenExtend = tpdp.RadialMenuEntryToggle("Sudden Extend", "TAG_INTERFACE_HELP")
		checkboxSuddenExtend.link_to_args(args, 1)
		checkboxSuddenExtend.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def SuddenExtendNewDay(attachee, args, evt_obj):
	#One charge per day
	args.set_arg(0, 1)
	
	#Set the checkbox to off at the begining of the day
	args.set_arg(1, 0)
	
	return 0

def OnMetamagicUpdate(attachee, args, evt_obj):
	
	#Check for a charge
	charges = args.get_arg(0)
	if charges < 1:
		return 0
	
	#Check if sudden extend is turned on
	if not args.get_arg(1):
		return 0
		
	metaMagicData = evt_obj.return_val
	
	#Unpack the extend count forom the metamagic data
	extendCount = (metaMagicData & 0xF000) >> 12
	
	#Don't Extend more than once
	if extendCount < 1:

		evt_obj.return_val = metaMagicData | (1 << 12)
	
		#Decriment the charges
		charges = charges - 1
		args.set_arg(0, charges)
	
	return 0


#Setup the feat
suddenExtendFeat = PythonModifier("Sudden Extend Feat", 4) #Charges, Toggeled On, Spare, Spare
suddenExtendFeat.MapToFeat("Sudden Extend")
suddenExtendFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, SuddenExtendRadial, ())
suddenExtendFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, SuddenExtendNewDay, ())
suddenExtendFeat.AddHook(ET_OnMetaMagicMod, EK_NONE, OnMetamagicUpdate, ())
