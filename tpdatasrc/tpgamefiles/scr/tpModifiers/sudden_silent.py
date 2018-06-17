from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Sudden Silent:  Complete Arcane, p. 83

print "Registering Sudden Silent"

def SuddenSilentRadial(attachee, args, evt_obj):
	#Add a checkbox to turn on and off sudden Silent if there is a carge available, otherwise just don't show it
	if args.get_arg(0):
		checkboxSuddenSilent = tpdp.RadialMenuEntryToggle("Sudden Silent", "TAG_INTERFACE_HELP")
		checkboxSuddenSilent.link_to_args(args, 1)
		checkboxSuddenSilent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def SuddenSilentNewDay(attachee, args, evt_obj):
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
	
	#Check if sudden silent is turned on
	if not args.get_arg(1):
		return 0
		
	#Get the metamagic info
	metaMagicData = evt_obj.meta_magic
	
	#Don't apply if it already has silent
	if not metaMagicData.get_silent():
		metaMagicData.set_silent(true)
	
		#Decriment the charges
		charges = charges - 1
		args.set_arg(0, charges)
	
	return 0


#Setup the feat
suddenSilentFeat = PythonModifier("Sudden Silent Feat", 4) #Charges, Toggeled On, Spare, Spare
suddenSilentFeat.MapToFeat("Sudden Silent")
suddenSilentFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, SuddenSilentRadial, ())
suddenSilentFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, SuddenSilentNewDay, ())
suddenSilentFeat.AddHook(ET_OnMetaMagicMod, EK_NONE, OnMetamagicUpdate, ())
