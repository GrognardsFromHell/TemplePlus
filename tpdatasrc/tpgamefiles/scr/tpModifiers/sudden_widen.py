from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Sudden Widen:  Complete Arcane, p. 83

print "Registering Sudden Widen"

def SuddenWidenRadial(attachee, args, evt_obj):
	#Add a checkbox to turn on and off sudden Widen if there is a carge available, otherwise just don't show it
	if args.get_arg(0):
		checkboxSuddenWiden = tpdp.RadialMenuEntryToggle("Sudden Widen", "TAG_INTERFACE_HELP")
		checkboxSuddenWiden.link_to_args(args, 1)
		checkboxSuddenWiden.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def SuddenWidenNewDay(attachee, args, evt_obj):
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
	
	#Check if sudden widen is turned on
	if not args.get_arg(1):
		return 0
	
	#Get the metamagic info
	metaMagicData = evt_obj.meta_magic
	
	#Don't widen more than once
	if metaMagicData.get_widen_count() < 1:
		metaMagicData.set_widen_count(1)
	
	return 0
	
	
def SuddenMetamagicDeductCharge(attachee, args, evt_obj):
	#Check for a charge and the enable flag
	charges = args.get_arg(0)
	if charges < 1 or not args.get_arg(1):
		return 0
		
	#Decriment the charges
	charges = charges - 1
	args.set_arg(0, charges)

	return 0

#Setup the feat
suddenWidenFeat = PythonModifier("Sudden Widen Feat", 4) #Charges, Toggeled On, Spare, Spare
suddenWidenFeat.MapToFeat("Sudden Widen")
suddenWidenFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, SuddenWidenRadial, ())
suddenWidenFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, SuddenWidenNewDay, ())
suddenWidenFeat.AddHook(ET_OnMetaMagicMod, EK_NONE, OnMetamagicUpdate, ())
suddenWidenFeat.AddHook(ET_OnD20PythonSignal, "Sudden Metamagic Deduct Charge", SuddenMetamagicDeductCharge, ())
